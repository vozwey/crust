/* crustcc: deliberately small Rust-like -> ISO C11 source translator.
 * Compiler implementation uses only the C standard library. gcc/clang are
 * separate backends, not used by this program. See README for the dialect.
 * Build: cc -std=c11 -Wall -Wextra -pedantic tools/crustcc.c -o crustcc
 * Run:   ./crustcc input.rs > output.c && cc -std=c11 output.c -o program
 */
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOLS 512
typedef struct { char *s; int line; } Token;
typedef struct { char *s; size_t n, cap; } Buf;
typedef struct { char name[128], type[128]; } Symbol;
static Token *tok;
static int nt, cap, at;
static const char *filename;
static Symbol types[MAX_SYMBOLS], vars[MAX_SYMBOLS], funcs[MAX_SYMBOLS], fields[MAX_SYMBOLS], variants[MAX_SYMBOLS];
static int ntypes, nvars, nfuncs, nfields, nvariants;
static char impl[128], result[128];

static void error_at(int index, const char *fmt, ...) {
    va_list args;
    fprintf(stderr, "%s:%d: error: ", filename, tok[index < nt ? index : nt-1].line);
    va_start(args, fmt); vfprintf(stderr, fmt, args); va_end(args);
    fputc('\n', stderr); exit(1);
}
static void *checked(void *p) { if (!p) { fputs("crustcc: out of memory\n", stderr); exit(1); } return p; }
static void add(Buf *b, const char *s) {
    size_t len = strlen(s);
    if (b->n + len + 1 > b->cap) {
        b->cap = (b->n + len + 1) * 2;
        b->s = checked(realloc(b->s, b->cap));
    }
    memcpy(b->s + b->n, s, len + 1); b->n += len;
}
static void addn(Buf *b, const char *s, size_t n) {
    char *p = checked(malloc(n+1)); memcpy(p, s, n); p[n] = 0; add(b, p); free(p);
}
static void emit_token(const char *p, size_t n, int line) {
    if (nt == cap) { cap = cap ? cap * 2 : 1024; tok = checked(realloc(tok, (size_t)cap * sizeof *tok)); }
    tok[nt].s = checked(malloc(n + 1)); memcpy(tok[nt].s, p, n); tok[nt].s[n] = 0;
    tok[nt++].line = line;
}
static int ident_start(int c) { return isalpha((unsigned char)c) || c == '_'; }
static int ident_char(int c) { return isalnum((unsigned char)c) || c == '_'; }
static void lex(const char *src) {
    int line = 1;
    for (const char *p = src; *p;) {
        if (isspace((unsigned char)*p)) { if (*p++ == '\n') line++; continue; }
        if (p[0]=='/' && p[1]=='/') { p+=2; while (*p && *p!='\n') p++; continue; }
        if (p[0]=='/' && p[1]=='*') {
            int start = line, level = 1; p += 2;
            while (*p && level) {
                if (p[0]=='/' && p[1]=='*') { level++; p+=2; }
                else if (p[0]=='*' && p[1]=='/') { level--; p+=2; }
                else { if (*p++ == '\n') line++; }
            }
            if (level) { emit_token("<unterminated comment>", 22, start); error_at(nt-1, "unterminated comment"); }
            continue;
        }
        if (ident_start(*p) || isdigit((unsigned char)*p)) {
            int number = isdigit((unsigned char)*p);
            const char *q = p++; while (ident_char(*p)) p++;
            if (number && *p == '.' && isdigit((unsigned char)p[1])) {
                p++; while (isdigit((unsigned char)*p)) p++;
            }
            emit_token(q, (size_t)(p-q), line); continue;
        }
        if (*p == '"' || *p == '\'') {
            int start = line; char quote = *p; const char *q = p++;
            while (*p && *p != quote) {
                if (*p == '\\' && p[1]) p++;
                if (*p++ == '\n') line++;
            }
            if (!*p) { emit_token("<unterminated string>", 21, start); error_at(nt-1, "unterminated string"); }
            p++; emit_token(q, (size_t)(p-q), start); continue;
        }
        if (!strncmp(p, "..=", 3)) { emit_token(p,3,line); p+=3; continue; }
        const char *two[] = {"->","=>","::","..","==","!=",">=","<=","&&","||","+=","-=","*=","/=","%=",NULL};
        int found = 0;
        for (int i=0; two[i]; i++) if (!strncmp(p,two[i],2)) { emit_token(p,2,line); p+=2; found=1; break; }
        if (found) continue;
        if (strchr("{}()[];:,.=+-*/%!&|<>?", *p)) { emit_token(p++,1,line); continue; }
        emit_token(p,1,line); error_at(nt-1, "unknown character '%c'", *p);
    }
    emit_token("<eof>", 5, line);
}
static int is(int i, const char *s) { return i >= 0 && i < nt && !strcmp(tok[i].s, s); }
static int take(const char *s) { if (is(at,s)) { at++; return 1; } return 0; }
static const char *need(const char *s) { if (!take(s)) error_at(at,"expected '%s', got '%s'", s,tok[at].s); return s; }
static const char *name(void) {
    if (!ident_start(tok[at].s[0])) error_at(at,"expected identifier, got '%s'",tok[at].s);
    return tok[at++].s;
}
static void symbol(Symbol *syms, int *n, const char *k, const char *v) {
    if (*n >= MAX_SYMBOLS) error_at(at,"too many symbols");
    if (strlen(k) >= sizeof syms[*n].name || strlen(v) >= sizeof syms[*n].type)
        error_at(at,"symbol name or type is too long");
    snprintf(syms[*n].name,128,"%s",k); snprintf(syms[*n].type,128,"%s",v); (*n)++;
}
static const char *lookup(Symbol *syms, int n, const char *k) {
    for (int i=n-1; i>=0; i--) if (!strcmp(syms[i].name,k)) return syms[i].type;
    return NULL;
}
static const char *c_type(const char *s) {
    const char *r[] = {"i8","int8_t","i16","int16_t","i32","int32_t","i64","int64_t",
        "u8","uint8_t","u16","uint16_t","u32","uint32_t","u64","uint64_t",
        "usize","size_t","isize","ptrdiff_t","f32","float","f64","double",
        "bool","bool","String","const char *","str","const char *","char","char",NULL};
    for (int i=0;r[i];i+=2) if (!strcmp(s,r[i])) return r[i+1];
    return s;
}
static const char *parse_type(void) {
    const char *s = name();
    if (is(at,"<") || is(at,"[") || is(at,"::")) error_at(at,"generic/compound types are not supported");
    return s;
}
static int pair(int start, const char *open, const char *close) {
    int d=0;
    for (int i=start;i<nt;i++) {
        if (is(i,open)) d++;
        if (is(i,close) && --d==0) return i;
        if (is(i,"<eof>")) break;
    }
    error_at(start,"unclosed '%s'",open); return start;
}
static int until(int start, const char *stop);
/* Expression rendering preserves precedence and only rewrites known dialect
 * constructs. Unknown method receivers are rejected rather than miscompiled. */
static void expr(Buf *b, int lo, int hi) {
    for (int i=lo; i<hi;) {
        const char *s=tok[i].s;
        if (is(i,"<eof>")) error_at(i,"incomplete expression");
        if (is(i,"(") || is(i,"[")) {
            const char *close = is(i,"(") ? ")" : "]";
            int end=pair(i,s,close);
            if (end>=hi) error_at(i,"unclosed expression");
            add(b,s); expr(b,i+1,end); add(b,close); i=end+1; continue;
        }
        if (is(i,"self") && is(i+1,".")) { add(b,"self->"); i+=2; continue; }
        if (ident_start((unsigned char)s[0]) && is(i+1,"::")) {
            const char *member = tok[i+2].s;
            if (!ident_start((unsigned char)member[0])) error_at(i+2,"expected associated name");
            if (is(i+3,"(")) { add(b,s); add(b,"_"); add(b,member); }
            else { /* Enum::Variant -> C enumerator */
                char key[256]; snprintf(key,sizeof key,"%s::%s",s,member);
                if (!lookup(variants,nvariants,key)) error_at(i,"unknown enum variant '%s'",key);
                add(b,member);
            }
            i+=3; continue;
        }
        if (ident_start((unsigned char)s[0]) && is(i+1,".") && is(i+3,"(")) {
            const char *ty=lookup(vars,nvars,s);
            if (!ty) error_at(i,"unknown method receiver '%s' (annotate its type)",s);
            char method[256]; snprintf(method,sizeof method,"%s_%s",ty,tok[i+2].s);
            if (!lookup(funcs,nfuncs,method)) error_at(i,"unknown method '%s'",method);
            int end=pair(i+3,"(",")"); if (end>=hi) error_at(i,"unclosed method call");
            add(b,method); add(b,"(&"); add(b,s);
            if (end>i+4) { add(b,", "); expr(b,i+4,end); }
            add(b,")"); i=end+1; continue;
        }
        if (lookup(types,ntypes,s) && is(i+1,"{")) {
            int end=pair(i+1,"{","}"); if (end>=hi) error_at(i,"unclosed struct literal");
            add(b,"("); add(b,s); add(b,"){ ");
            int j=i+2;
            while (j<end) {
                const char *field=tok[j++].s;
                if (!ident_start((unsigned char)field[0]) || !is(j,":"))
                    error_at(j,"expected 'field: value' in struct literal");
                j++;
                int value=j, stop=until(j,",");
                if (stop>end) error_at(j,"unclosed struct field");
                add(b,"."); add(b,field); add(b," = "); expr(b,value,stop);
                if (is(stop,",")) { add(b,", "); j=stop+1; }
                else j=stop;
            }
            add(b," }"); i=end+1; continue;
        }
        if (is(i,"as")) {
            if (!b->n || i+1>=hi || !ident_start((unsigned char)tok[i+1].s[0]))
                error_at(i,"expected expression 'as' Type");
            /* Parenthesize the expression so method calls and field access cast
             * correctly. Require parentheses for a binary expression before as. */
            int nested=0;
            for (int j=lo;j<i;j++) {
                if (is(j,"(")) nested++;
                else if (is(j,")")) nested--;
                else if (!nested && (is(j,"+") || is(j,"-") || is(j,"*") || is(j,"/")))
                    error_at(i,"parenthesize binary expression before 'as'");
            }
            char *old=checked(malloc(b->n+1)); strcpy(old,b->s);
            b->n=0; b->s[0]=0;
            add(b,"(("); add(b,c_type(tok[i+1].s)); add(b,")("); add(b,old); add(b,")) ");
            free(old); i+=2; continue;
        }
        if (is(i,"{") || is(i,"}") || is(i,"..") || is(i,"..=") ||
            is(i,"=>") || is(i,"::") || is(i,"?"))
            error_at(i,"unsupported expression token '%s'",s);
        add(b,s); add(b," "); i++;
    }
}
static char *expression(int lo, int hi) {
    Buf b={0}; expr(&b,lo,hi); if (!b.s) add(&b,""); return b.s;
}
/* Find a top-level delimiter without stopping inside nested calls/literals. */
static int until(int start, const char *stop) {
    int par=0, brace=0, square=0;
    for (int i=start; i<nt; i++) {
        if (par==0 && brace==0 && square==0 && is(i,stop)) return i;
        if (is(i,"(")) par++; else if (is(i,")")) par--;
        else if (is(i,"{")) brace++; else if (is(i,"}")) { if (!brace) return i; brace--; }
        else if (is(i,"[")) square++; else if (is(i,"]")) square--;
        if (is(i,"<eof>")) break;
    }
    error_at(start,"expected '%s'",stop); return start;
}
static const char *infer(int lo, int hi) {
    if (hi<=lo) return NULL;
    const char *s=tok[lo].s;
    int nesting=0;
    for (int j=lo;j<hi;j++) {
        if (is(j,"(")) nesting++;
        else if (is(j,")")) nesting--;
        else if (!nesting && is(j,"as") && j+1<hi) return tok[j+1].s;
    }
    if (is(lo,"true") || is(lo,"false")) return "bool";
    if (*s=='"') return "str";
    if (*s=='\'') return "char";
    if (isdigit((unsigned char)*s)) return strchr(s,'.') ? "f64" : "i32";
    if (is(lo,"-") && lo+1<hi && isdigit((unsigned char)tok[lo+1].s[0]))
        return strchr(tok[lo+1].s,'.') ? "f64" : "i32";
    if (is(lo+1,"::")) {
        if (is(lo+3,"(")) {
            char key[256]; snprintf(key,sizeof key,"%s_%s",s,tok[lo+2].s);
            const char *ty=lookup(funcs,nfuncs,key); if (ty) return ty;
        }
        if (lookup(types,ntypes,s)) return s;
    }
    if (is(lo+1,".") && is(lo+3,"(")) {
        const char *ty=lookup(vars,nvars,s);
        if (ty) { char key[256]; snprintf(key,sizeof key,"%s_%s",ty,tok[lo+2].s);
            return lookup(funcs,nfuncs,key); }
    }
    if (is(lo+1,"(") ) return lookup(funcs,nfuncs,s);
    if (is(lo+1,"{")) return lookup(types,ntypes,s) ? s : NULL;
    if (is(lo+1,".") && lo+2<hi) {
        const char *ty=lookup(vars,nvars,s);
        if (ty) { char key[256]; snprintf(key,sizeof key,"%s.%s",ty,tok[lo+2].s);
            return lookup(fields,nfields,key); }
    }
    if (is(lo+1,"as") && lo+2<hi) return tok[lo+2].s;
    return lookup(vars,nvars,s);
}
static void print_call(const char *macro);
static void statement(void);
static void block(void) {
    need("{"); puts("{");
    int oldvars=nvars;
    while (!is(at,"}")) { if (is(at,"<eof>")) error_at(at,"unclosed block"); statement(); }
    need("}"); puts("}"); nvars=oldvars;
}
static void statement(void) {
    if (take("let")) {
        take("mut"); const char *v=name(); const char *ty=NULL;
        if (take(":")) ty=parse_type();
        need("="); int lo=at, end=until(at,";"); at=end+1;
        if (!ty) ty=infer(lo,end);
        if (!ty) error_at(lo,"cannot infer type of '%s'; add ': Type'",v);
        char *rhs=expression(lo,end);
        printf("    %s %s = %s;\n", c_type(ty),v,rhs); free(rhs);
        symbol(vars,&nvars,v,ty); return;
    }
    if (is(at,"println") || is(at,"print") || is(at,"eprintln")) {
        const char *macro=name(); need("!"); print_call(macro); need(";"); return;
    }
    if (take("loop")) { fputs("    while (1) ",stdout); block(); return; }
    if (take("for")) {
        const char *v=name(); need("in"); int lo=at;
        while (!is(at,"..") && !is(at,"..=") && !is(at,"<eof>")) at++;
        if (is(at,"<eof>")) error_at(at,"expected range '..' or '..='");
        int inclusive=take("..="); if (!inclusive) need(".."); int hi=at;
        while (!is(at,"{") && !is(at,"<eof>")) at++;
        if (is(at,"<eof>")) error_at(at,"expected '{' after range");
        char *start=expression(lo,hi-1), *end=expression(hi,at);
        printf("    for (int32_t %s = %s; %s %s %s; %s++) ",v,start,v,inclusive?"<=":"<",end,v);
        free(start); free(end);
        int old=nvars; symbol(vars,&nvars,v,"i32"); block(); nvars=old; return;
    }
    if (take("if") || take("while")) {
        const char *word=tok[at-1].s; int lo=at;
        while (!is(at,"{") && !is(at,"<eof>")) at++;
        char *cond=expression(lo,at);
        printf("    %s (%s) ",word,cond); free(cond); block();
        if (take("else")) { fputs("    else ",stdout); block(); }
        return;
    }
    if (take("match")) {
        int lo=at;
        while (!is(at,"{") && !is(at,"<eof>")) at++;
        char *subject=expression(lo,at);
        printf("    switch (%s) {\n",subject); free(subject); need("{");
        int has_default=0;
        while (!is(at,"}")) {
            if (take("_")) { fputs("    default: ",stdout); has_default=1; }
            else { const char *type=name(); need("::"); const char *variant=name();
                char key[256]; snprintf(key,sizeof key,"%s::%s",type,variant);
                if (!lookup(variants,nvariants,key)) error_at(at,"unknown enum variant '%s'",key);
                printf("    case %s: ",variant); }
            need("=>"); int arm=at;
            int end=until(at,",");
            at=end + (is(end,",") ? 1 : 0);
            if (!strcmp(result,"void")) error_at(arm,"value match requires a returning function");
            char *value=expression(arm,end); printf("return %s;\n",value); free(value);
        }
        if (!has_default) error_at(at,"value match requires a '_' fallback arm");
        need("}"); puts("    }"); return;
    }
    if (take("return")) {
        int lo=at,end=until(at,";"); at=end+1; char *e=expression(lo,end);
        printf("    return %s;\n",e); free(e); return;
    }
    if (take("break")) { need(";"); puts("    break;"); return; }
    if (take("continue")) { need(";"); puts("    continue;"); return; }
    if (is(at,"{")) { block(); return; }
    int lo=at, end=until(at,";");
    int tail=is(end,"}");
    if (tail && !strcmp(result,"void")) error_at(lo,"trailing expression in a void function");
    if (end==lo) error_at(lo,"empty statement");
    char *e=expression(lo,end);
    if (tail) { printf("    return %s;\n",e); at=end; }
    else { printf("    %s;\n",e); at=end+1; }
    free(e);
}
static void print_call(const char *macro) {
    need("(");
    if (tok[at].s[0]!='"') error_at(at,"first print argument must be a string literal");
    const char *format=tok[at++].s;
    int args[128], ends[128], count=0;
    while (take(",")) {
        if (count==128) error_at(at,"too many print arguments");
        args[count]=at;
        while (!is(at,",") && !is(at,")") && !is(at,"<eof>")) {
            if (is(at,"(")) at=pair(at,"(",")")+1;
            else if (is(at,"{")) at=pair(at,"{","}")+1;
            else if (is(at,"[")) at=pair(at,"[","]")+1;
            else at++;
        }
        ends[count++]=at;
    }
    need(")");
    int newline=strcmp(macro,"print")!=0;
    fputs(!strcmp(macro,"eprintln")?"    fprintf(stderr, \"":"    printf(\"",stdout);
    int used=0;
    const char *argtype[128]={0};
    for (const char *p=format+1; p[0] && p[1]; p++) {
        if (p[0]=='{' && p[1]=='{') { fputc('{',stdout); p++; continue; }
        if (p[0]=='}' && p[1]=='}') { fputc('}',stdout); p++; continue; }
        if (p[0]=='{' && p[1]=='}') {
            if (used==count) error_at(at,"too few arguments for print format");
            const char *ty=infer(args[used],ends[used]);
            if (!ty) error_at(args[used],"cannot infer print argument type");
            argtype[used]=ty;
            if (!strcmp(ty,"str") || !strcmp(ty,"String")) fputs("%s",stdout);
            else if (!strcmp(ty,"f32") || !strcmp(ty,"f64")) fputs("%f",stdout);
            else if (!strcmp(ty,"i64")) fputs("%lld",stdout);
            else if (!strcmp(ty,"u64")) fputs("%llu",stdout);
            else if (!strcmp(ty,"usize")) fputs("%zu",stdout);
            else if (!strcmp(ty,"isize")) fputs("%td",stdout);
            else if (!strcmp(ty,"u32")) fputs("%u",stdout);
            else if (!strcmp(ty,"char")) fputs("%c",stdout);
            else if (!strcmp(ty,"i8") || !strcmp(ty,"i16") || !strcmp(ty,"i32") ||
                     !strcmp(ty,"u8") || !strcmp(ty,"u16") || !strcmp(ty,"bool")) fputs("%d",stdout);
            else error_at(args[used],"print does not support type '%s'",ty);
            used++; p++; continue;
        }
        if (*p=='{' || *p=='}') error_at(at,"only {} placeholders are supported");
        if (*p=='%') fputc('%',stdout);
        fputc(*p,stdout);
    }
    if (used!=count) error_at(at,"too many arguments for print format");
    if (newline) fputs("\\n",stdout);
    fputc('"',stdout);
    for (int j=0;j<count;j++) {
        char *e=expression(args[j],ends[j]);
        const char *cast = !strcmp(argtype[j],"i64") ? "long long" :
                           !strcmp(argtype[j],"u64") ? "unsigned long long" :
                           !strcmp(argtype[j],"u32") ? "unsigned int" : NULL;
        if (cast) printf(", (%s)(%s)",cast,e);
        else printf(", %s",e);
        free(e);
    }
    puts(");");
}
static void function(void) {
    need("fn"); const char *n=name();
    char fullname[256]; snprintf(fullname,sizeof fullname,"%s%s%s",impl,impl[0]?"_":"",n);
    int mainfn=!impl[0] && !strcmp(n,"main");
    need("(");
    char params[2048]="";
    int old=nvars; nvars=0;
    if (!is(at,")")) do {
        const char *v,*ty;
        if (take("&")) { take("mut"); need("self");
            if (!impl[0]) error_at(at,"&self outside impl");
            v="self"; ty=impl;
            if (params[0]) strcat(params,", ");
            strcat(params,impl); strcat(params," *self");
        } else {
            take("mut"); v=name(); need(":"); ty=parse_type();
            if (params[0]) strcat(params,", ");
            if (strlen(params)+strlen(ty)+strlen(v)+5>=sizeof params) error_at(at,"too many parameters");
            strcat(params,c_type(ty)); strcat(params," "); strcat(params,v);
        }
        symbol(vars,&nvars,v,ty);
    } while (take(",") && !is(at,")"));
    need(")");
    const char *ty=take("->")?parse_type():(mainfn?"i32":"void");
    snprintf(result,sizeof result,"%s",ty);
    symbol(funcs,&nfuncs,fullname,ty);
    printf("%s %s(%s) ",c_type(ty),fullname,params[0]?params:"void");
    block();
    nvars=old;
}
static void definition(void) {
    while (take("pub") || take("unsafe") || take("async")) { /* syntax only; no async runtime */ }
    if (is(at,"fn")) { function(); return; }
    if (take("struct")) {
        const char *n=name(); symbol(types,&ntypes,n,"struct");
        need("{"); printf("typedef struct %s {\n",n);
        while (!is(at,"}")) {
            const char *field=name(); need(":"); const char *ty=parse_type();
            char key[256]; snprintf(key,sizeof key,"%s.%s",n,field);
            symbol(fields,&nfields,key,ty);
            printf("    %s %s;\n",c_type(ty),field); if (!take(",")) break;
        }
        need("}"); take(";"); printf("} %s;\n",n); return;
    }
    if (take("enum")) {
        const char *n=name(); symbol(types,&ntypes,n,"enum");
        need("{"); printf("typedef enum %s {\n",n);
        while (!is(at,"}")) {
            const char *variant=name(); char key[256];
            snprintf(key,sizeof key,"%s::%s",n,variant); symbol(variants,&nvariants,key,n);
            printf("    %s,\n",variant); if (!take(",")) break;
        }
        need("}"); take(";"); printf("} %s;\n",n); return;
    }
    if (take("impl")) {
        snprintf(impl,sizeof impl,"%s",name());
        if (!lookup(types,ntypes,impl)) error_at(at,"impl of unknown type '%s'",impl);
        need("{"); while (!is(at,"}")) { if (is(at,"<eof>")) error_at(at,"unclosed impl"); definition(); }
        need("}"); impl[0]=0; return;
    }
    error_at(at,"unsupported top-level construct '%s'",tok[at].s);
}
int main(int argc, char **argv) {
    if (argc!=2) { fputs("usage: crustcc input.rs > output.c\n",stderr); return 2; }
    filename=argv[1]; FILE *f=fopen(filename,"rb"); if (!f) { perror(filename); return 1; }
    Buf src={0}; char buffer[4096]; size_t n;
    while ((n=fread(buffer,1,sizeof buffer,f))!=0) addn(&src,buffer,n);
    if (ferror(f)) { perror(filename); fclose(f); return 1; }
    fclose(f); if (!src.s) add(&src,"");
    lex(src.s);
    puts("/* Generated by crustcc; ISO C11. */\n#include <stdio.h>\n#include <stdint.h>\n#include <stddef.h>\n#include <stdbool.h>");
    while (!is(at,"<eof>")) definition();
    for (int i=0;i<nt;i++) free(tok[i].s);
    free(tok); free(src.s);
    return 0;
}

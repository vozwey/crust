/* ============================================================================
 * crustc.h — "crustc": a tiny COMPILER that runs entirely inside the C
 * preprocessor, built on Paul Fultz II's deferred-evaluation technique
 * (https://pfultz2.com/blog/2012/05/10/turing/):
 *
 *   EVAL     — drives thousands of rescans of the macro expander
 *   DEFER/OBSTRUCT — defer recursive calls past the "blue paint" so
 *              recursion actually progresses across scans
 *   CHECK/NOT/IIF  — boolean logic on decimal tokens
 *   INC/DEC  — arithmetic via token-pasting lookup tables
 *
 * Result: CRUST_ADD(40, 2) expands to the literal token `42` BEFORE gcc
 * ever parses the file — usable in static_assert, enum, array sizes.
 *
 * (A full Rust compiler is still impossible in cpp: it can only rewrite
 * identifier tokens and cannot read files. But arithmetic — it can do.)
 * ==========================================================================*/
#ifndef CRUSTC_H
#define CRUSTC_H

/* ---------- deferred evaluation (Fultz II style) ------------------------- */
#define CRUST_EMPTY()
#define CRUST_DEFER(id) id CRUST_EMPTY()
#define CRUST_OBSTRUCT(...) __VA_ARGS__ CRUST_DEFER(CRUST_EMPTY)()
#define CRUST_EXPAND(...) __VA_ARGS__
#define CRUST_EAT(...)

#define CRUST_EVAL(...)   CRUST_EVAL0(__VA_ARGS__)
#define CRUST_EVAL0(...)  CRUST_EVAL1(CRUST_EVAL1(CRUST_EVAL1(__VA_ARGS__)))
#define CRUST_EVAL1(...)  CRUST_EVAL2(CRUST_EVAL2(CRUST_EVAL2(__VA_ARGS__)))
#define CRUST_EVAL2(...)  CRUST_EVAL3(CRUST_EVAL3(CRUST_EVAL3(__VA_ARGS__)))
#define CRUST_EVAL3(...)  CRUST_EVAL4(CRUST_EVAL4(CRUST_EVAL4(__VA_ARGS__)))
#define CRUST_EVAL4(...)  CRUST_EVAL5(CRUST_EVAL5(CRUST_EVAL5(__VA_ARGS__)))
#define CRUST_EVAL5(...)  CRUST_EVAL6(CRUST_EVAL6(CRUST_EVAL6(__VA_ARGS__)))
#define CRUST_EVAL6(...)  CRUST_EVAL7(CRUST_EVAL7(CRUST_EVAL7(__VA_ARGS__)))
#define CRUST_EVAL7(...)  CRUST_EVAL8(CRUST_EVAL8(CRUST_EVAL8(__VA_ARGS__)))
#define CRUST_EVAL8(...)  __VA_ARGS__
/* 3^8 = 6561 rescans per CRUST_EVAL */

/* ---------- boolean logic on decimal tokens ------------------------------ */
#define CRUST_CAT(a, b)   CRUST_PRIMITIVE_CAT(a, b)
#define CRUST_PRIMITIVE_CAT(a, b) a ## b
#define CRUST_CHECK_N(x, n, ...) n
#define CRUST_CHECK(...)  CRUST_CHECK_N(__VA_ARGS__, 0,)
#define CRUST_NOT(x)      CRUST_CHECK(CRUST_CAT(CRUST_NOT_, x))
#define CRUST_NOT_0       ~, 1,
#define CRUST_BOOL(x)     CRUST_NOT(CRUST_NOT(x))
#define CRUST_IIF(c)      CRUST_CAT(CRUST_IIF_, c)
#define CRUST_IIF_0(t, ...) __VA_ARGS__
#define CRUST_IIF_1(t, ...) t
#define CRUST_IF(c)       CRUST_IIF(CRUST_BOOL(c))
#define CRUST_WHEN(c)     CRUST_IF(c)(CRUST_EXPAND, CRUST_EAT)

/* ---------- INC/DEC via token-pasting tables (generated below, 0..200) --- */
#define CRUST_INC(x) CRUST_CAT(CRUST_INC_, x)
#define CRUST_DEC(x) CRUST_CAT(CRUST_DEC_, x)

/* ADD(a,b) at preprocess time: apply INC to a exactly b times.
 * Verified fast for small numbers, e.g. CRUST_ADD(40, 2) -> 42. */
#define CRUST_ADD(a, b)     CRUST_EVAL(CRUST_ADD_ITER(b, a))
#define CRUST_ADD_ITER(b, acc) \
    CRUST_IF(b)(CRUST_OBSTRUCT(CRUST_ADD_ITER_INDIRECT)()(CRUST_DEC(b), CRUST_INC(acc)), acc)
#define CRUST_ADD_ITER_INDIRECT() CRUST_ADD_ITER

/* REPEAT(count, macro, ...): the canonical demo from the Fultz post —
 * streams `macro(0) macro(1) ... macro(count-1)` into the token stream. */
#define CRUST_REPEAT(count, macro, ...) \
    CRUST_WHEN(count)( \
        CRUST_OBSTRUCT(CRUST_REPEAT_INDIRECT)()(CRUST_DEC(count), macro, __VA_ARGS__), \
        CRUST_OBSTRUCT(macro)(CRUST_DEC(count), __VA_ARGS__) \
    )
#define CRUST_REPEAT_INDIRECT() CRUST_REPEAT

/* ---------- rusty aliases ------------------------------------------------- */
#define add(a, b) CRUST_ADD(a, b)

#define CRUST_INC_0 1
#define CRUST_INC_1 2
#define CRUST_INC_2 3
#define CRUST_INC_3 4
#define CRUST_INC_4 5
#define CRUST_INC_5 6
#define CRUST_INC_6 7
#define CRUST_INC_7 8
#define CRUST_INC_8 9
#define CRUST_INC_9 10
#define CRUST_INC_10 11
#define CRUST_INC_11 12
#define CRUST_INC_12 13
#define CRUST_INC_13 14
#define CRUST_INC_14 15
#define CRUST_INC_15 16
#define CRUST_INC_16 17
#define CRUST_INC_17 18
#define CRUST_INC_18 19
#define CRUST_INC_19 20
#define CRUST_INC_20 21
#define CRUST_INC_21 22
#define CRUST_INC_22 23
#define CRUST_INC_23 24
#define CRUST_INC_24 25
#define CRUST_INC_25 26
#define CRUST_INC_26 27
#define CRUST_INC_27 28
#define CRUST_INC_28 29
#define CRUST_INC_29 30
#define CRUST_INC_30 31
#define CRUST_INC_31 32
#define CRUST_INC_32 33
#define CRUST_INC_33 34
#define CRUST_INC_34 35
#define CRUST_INC_35 36
#define CRUST_INC_36 37
#define CRUST_INC_37 38
#define CRUST_INC_38 39
#define CRUST_INC_39 40
#define CRUST_INC_40 41
#define CRUST_INC_41 42
#define CRUST_INC_42 43
#define CRUST_INC_43 44
#define CRUST_INC_44 45
#define CRUST_INC_45 46
#define CRUST_INC_46 47
#define CRUST_INC_47 48
#define CRUST_INC_48 49
#define CRUST_INC_49 50
#define CRUST_INC_50 51
#define CRUST_INC_51 52
#define CRUST_INC_52 53
#define CRUST_INC_53 54
#define CRUST_INC_54 55
#define CRUST_INC_55 56
#define CRUST_INC_56 57
#define CRUST_INC_57 58
#define CRUST_INC_58 59
#define CRUST_INC_59 60
#define CRUST_INC_60 61
#define CRUST_INC_61 62
#define CRUST_INC_62 63
#define CRUST_INC_63 64
#define CRUST_INC_64 65
#define CRUST_INC_65 66
#define CRUST_INC_66 67
#define CRUST_INC_67 68
#define CRUST_INC_68 69
#define CRUST_INC_69 70
#define CRUST_INC_70 71
#define CRUST_INC_71 72
#define CRUST_INC_72 73
#define CRUST_INC_73 74
#define CRUST_INC_74 75
#define CRUST_INC_75 76
#define CRUST_INC_76 77
#define CRUST_INC_77 78
#define CRUST_INC_78 79
#define CRUST_INC_79 80
#define CRUST_INC_80 81
#define CRUST_INC_81 82
#define CRUST_INC_82 83
#define CRUST_INC_83 84
#define CRUST_INC_84 85
#define CRUST_INC_85 86
#define CRUST_INC_86 87
#define CRUST_INC_87 88
#define CRUST_INC_88 89
#define CRUST_INC_89 90
#define CRUST_INC_90 91
#define CRUST_INC_91 92
#define CRUST_INC_92 93
#define CRUST_INC_93 94
#define CRUST_INC_94 95
#define CRUST_INC_95 96
#define CRUST_INC_96 97
#define CRUST_INC_97 98
#define CRUST_INC_98 99
#define CRUST_INC_99 100
#define CRUST_INC_100 101
#define CRUST_INC_101 102
#define CRUST_INC_102 103
#define CRUST_INC_103 104
#define CRUST_INC_104 105
#define CRUST_INC_105 106
#define CRUST_INC_106 107
#define CRUST_INC_107 108
#define CRUST_INC_108 109
#define CRUST_INC_109 110
#define CRUST_INC_110 111
#define CRUST_INC_111 112
#define CRUST_INC_112 113
#define CRUST_INC_113 114
#define CRUST_INC_114 115
#define CRUST_INC_115 116
#define CRUST_INC_116 117
#define CRUST_INC_117 118
#define CRUST_INC_118 119
#define CRUST_INC_119 120
#define CRUST_INC_120 121
#define CRUST_INC_121 122
#define CRUST_INC_122 123
#define CRUST_INC_123 124
#define CRUST_INC_124 125
#define CRUST_INC_125 126
#define CRUST_INC_126 127
#define CRUST_INC_127 128
#define CRUST_INC_128 129
#define CRUST_INC_129 130
#define CRUST_INC_130 131
#define CRUST_INC_131 132
#define CRUST_INC_132 133
#define CRUST_INC_133 134
#define CRUST_INC_134 135
#define CRUST_INC_135 136
#define CRUST_INC_136 137
#define CRUST_INC_137 138
#define CRUST_INC_138 139
#define CRUST_INC_139 140
#define CRUST_INC_140 141
#define CRUST_INC_141 142
#define CRUST_INC_142 143
#define CRUST_INC_143 144
#define CRUST_INC_144 145
#define CRUST_INC_145 146
#define CRUST_INC_146 147
#define CRUST_INC_147 148
#define CRUST_INC_148 149
#define CRUST_INC_149 150
#define CRUST_INC_150 151
#define CRUST_INC_151 152
#define CRUST_INC_152 153
#define CRUST_INC_153 154
#define CRUST_INC_154 155
#define CRUST_INC_155 156
#define CRUST_INC_156 157
#define CRUST_INC_157 158
#define CRUST_INC_158 159
#define CRUST_INC_159 160
#define CRUST_INC_160 161
#define CRUST_INC_161 162
#define CRUST_INC_162 163
#define CRUST_INC_163 164
#define CRUST_INC_164 165
#define CRUST_INC_165 166
#define CRUST_INC_166 167
#define CRUST_INC_167 168
#define CRUST_INC_168 169
#define CRUST_INC_169 170
#define CRUST_INC_170 171
#define CRUST_INC_171 172
#define CRUST_INC_172 173
#define CRUST_INC_173 174
#define CRUST_INC_174 175
#define CRUST_INC_175 176
#define CRUST_INC_176 177
#define CRUST_INC_177 178
#define CRUST_INC_178 179
#define CRUST_INC_179 180
#define CRUST_INC_180 181
#define CRUST_INC_181 182
#define CRUST_INC_182 183
#define CRUST_INC_183 184
#define CRUST_INC_184 185
#define CRUST_INC_185 186
#define CRUST_INC_186 187
#define CRUST_INC_187 188
#define CRUST_INC_188 189
#define CRUST_INC_189 190
#define CRUST_INC_190 191
#define CRUST_INC_191 192
#define CRUST_INC_192 193
#define CRUST_INC_193 194
#define CRUST_INC_194 195
#define CRUST_INC_195 196
#define CRUST_INC_196 197
#define CRUST_INC_197 198
#define CRUST_INC_198 199
#define CRUST_INC_199 200
#define CRUST_INC_200 201
#define CRUST_DEC_0 0
#define CRUST_DEC_1 0
#define CRUST_DEC_2 1
#define CRUST_DEC_3 2
#define CRUST_DEC_4 3
#define CRUST_DEC_5 4
#define CRUST_DEC_6 5
#define CRUST_DEC_7 6
#define CRUST_DEC_8 7
#define CRUST_DEC_9 8
#define CRUST_DEC_10 9
#define CRUST_DEC_11 10
#define CRUST_DEC_12 11
#define CRUST_DEC_13 12
#define CRUST_DEC_14 13
#define CRUST_DEC_15 14
#define CRUST_DEC_16 15
#define CRUST_DEC_17 16
#define CRUST_DEC_18 17
#define CRUST_DEC_19 18
#define CRUST_DEC_20 19
#define CRUST_DEC_21 20
#define CRUST_DEC_22 21
#define CRUST_DEC_23 22
#define CRUST_DEC_24 23
#define CRUST_DEC_25 24
#define CRUST_DEC_26 25
#define CRUST_DEC_27 26
#define CRUST_DEC_28 27
#define CRUST_DEC_29 28
#define CRUST_DEC_30 29
#define CRUST_DEC_31 30
#define CRUST_DEC_32 31
#define CRUST_DEC_33 32
#define CRUST_DEC_34 33
#define CRUST_DEC_35 34
#define CRUST_DEC_36 35
#define CRUST_DEC_37 36
#define CRUST_DEC_38 37
#define CRUST_DEC_39 38
#define CRUST_DEC_40 39
#define CRUST_DEC_41 40
#define CRUST_DEC_42 41
#define CRUST_DEC_43 42
#define CRUST_DEC_44 43
#define CRUST_DEC_45 44
#define CRUST_DEC_46 45
#define CRUST_DEC_47 46
#define CRUST_DEC_48 47
#define CRUST_DEC_49 48
#define CRUST_DEC_50 49
#define CRUST_DEC_51 50
#define CRUST_DEC_52 51
#define CRUST_DEC_53 52
#define CRUST_DEC_54 53
#define CRUST_DEC_55 54
#define CRUST_DEC_56 55
#define CRUST_DEC_57 56
#define CRUST_DEC_58 57
#define CRUST_DEC_59 58
#define CRUST_DEC_60 59
#define CRUST_DEC_61 60
#define CRUST_DEC_62 61
#define CRUST_DEC_63 62
#define CRUST_DEC_64 63
#define CRUST_DEC_65 64
#define CRUST_DEC_66 65
#define CRUST_DEC_67 66
#define CRUST_DEC_68 67
#define CRUST_DEC_69 68
#define CRUST_DEC_70 69
#define CRUST_DEC_71 70
#define CRUST_DEC_72 71
#define CRUST_DEC_73 72
#define CRUST_DEC_74 73
#define CRUST_DEC_75 74
#define CRUST_DEC_76 75
#define CRUST_DEC_77 76
#define CRUST_DEC_78 77
#define CRUST_DEC_79 78
#define CRUST_DEC_80 79
#define CRUST_DEC_81 80
#define CRUST_DEC_82 81
#define CRUST_DEC_83 82
#define CRUST_DEC_84 83
#define CRUST_DEC_85 84
#define CRUST_DEC_86 85
#define CRUST_DEC_87 86
#define CRUST_DEC_88 87
#define CRUST_DEC_89 88
#define CRUST_DEC_90 89
#define CRUST_DEC_91 90
#define CRUST_DEC_92 91
#define CRUST_DEC_93 92
#define CRUST_DEC_94 93
#define CRUST_DEC_95 94
#define CRUST_DEC_96 95
#define CRUST_DEC_97 96
#define CRUST_DEC_98 97
#define CRUST_DEC_99 98
#define CRUST_DEC_100 99
#define CRUST_DEC_101 100
#define CRUST_DEC_102 101
#define CRUST_DEC_103 102
#define CRUST_DEC_104 103
#define CRUST_DEC_105 104
#define CRUST_DEC_106 105
#define CRUST_DEC_107 106
#define CRUST_DEC_108 107
#define CRUST_DEC_109 108
#define CRUST_DEC_110 109
#define CRUST_DEC_111 110
#define CRUST_DEC_112 111
#define CRUST_DEC_113 112
#define CRUST_DEC_114 113
#define CRUST_DEC_115 114
#define CRUST_DEC_116 115
#define CRUST_DEC_117 116
#define CRUST_DEC_118 117
#define CRUST_DEC_119 118
#define CRUST_DEC_120 119
#define CRUST_DEC_121 120
#define CRUST_DEC_122 121
#define CRUST_DEC_123 122
#define CRUST_DEC_124 123
#define CRUST_DEC_125 124
#define CRUST_DEC_126 125
#define CRUST_DEC_127 126
#define CRUST_DEC_128 127
#define CRUST_DEC_129 128
#define CRUST_DEC_130 129
#define CRUST_DEC_131 130
#define CRUST_DEC_132 131
#define CRUST_DEC_133 132
#define CRUST_DEC_134 133
#define CRUST_DEC_135 134
#define CRUST_DEC_136 135
#define CRUST_DEC_137 136
#define CRUST_DEC_138 137
#define CRUST_DEC_139 138
#define CRUST_DEC_140 139
#define CRUST_DEC_141 140
#define CRUST_DEC_142 141
#define CRUST_DEC_143 142
#define CRUST_DEC_144 143
#define CRUST_DEC_145 144
#define CRUST_DEC_146 145
#define CRUST_DEC_147 146
#define CRUST_DEC_148 147
#define CRUST_DEC_149 148
#define CRUST_DEC_150 149
#define CRUST_DEC_151 150
#define CRUST_DEC_152 151
#define CRUST_DEC_153 152
#define CRUST_DEC_154 153
#define CRUST_DEC_155 154
#define CRUST_DEC_156 155
#define CRUST_DEC_157 156
#define CRUST_DEC_158 157
#define CRUST_DEC_159 158
#define CRUST_DEC_160 159
#define CRUST_DEC_161 160
#define CRUST_DEC_162 161
#define CRUST_DEC_163 162
#define CRUST_DEC_164 163
#define CRUST_DEC_165 164
#define CRUST_DEC_166 165
#define CRUST_DEC_167 166
#define CRUST_DEC_168 167
#define CRUST_DEC_169 168
#define CRUST_DEC_170 169
#define CRUST_DEC_171 170
#define CRUST_DEC_172 171
#define CRUST_DEC_173 172
#define CRUST_DEC_174 173
#define CRUST_DEC_175 174
#define CRUST_DEC_176 175
#define CRUST_DEC_177 176
#define CRUST_DEC_178 177
#define CRUST_DEC_179 178
#define CRUST_DEC_180 179
#define CRUST_DEC_181 180
#define CRUST_DEC_182 181
#define CRUST_DEC_183 182
#define CRUST_DEC_184 183
#define CRUST_DEC_185 184
#define CRUST_DEC_186 185
#define CRUST_DEC_187 186
#define CRUST_DEC_188 187
#define CRUST_DEC_189 188
#define CRUST_DEC_190 189
#define CRUST_DEC_191 190
#define CRUST_DEC_192 191
#define CRUST_DEC_193 192
#define CRUST_DEC_194 193
#define CRUST_DEC_195 194
#define CRUST_DEC_196 195
#define CRUST_DEC_197 196
#define CRUST_DEC_198 197
#define CRUST_DEC_199 198
#define CRUST_DEC_200 199
#endif /* CRUSTC_H */

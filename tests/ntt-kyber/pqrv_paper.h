#ifndef PQRV_PAPER_H
#define PQRV_PAPER_H
#include "kyber.h"
#include <stdint.h>

/* Zetas / shuffle-mask table consumed by the Kyber RVV NTT, INTT and order
 * conversion kernels. This is PQRV's `qdata_vlen128` (ntt/kyber/consts_vlen128.c)
 * verbatim: the element offsets below match the KYBER_NTT_RVV_VLEN128_CONSTS_H
 * header that the assembly indexes with (`_MASK_*`, `_ZETAS_*`, `_ZETA_EXP_INTT_*`).
 * Using the real table rather than a zero array keeps the kernels away from
 * degenerate inputs. */
#define I16(l, h) (h << 8) | l

const int16_t zetas[] __attribute__((aligned(16))) = {
#define _MASK_45674567
    4,           5,           6,           7,           4,
    5,           6,           7,
#define _MASK_01230123
    0,           1,           2,           3,           0,
    1,           2,           3,
#define _MASK_01014545
    0,           1,           0,           1,           4,
    5,           4,           5,
#define _MASK_23236767
    2,           3,           2,           3,           6,
    7,           6,           7,
#define _MASK_10325476
    1,           0,           3,           2,           5,
    4,           7,           6,
#define _REJ_UNIFORM_IDX8
    I16(0, 1),   I16(1, 2),   I16(3, 4),   I16(4, 5),   I16(6, 7),
    I16(7, 8),   I16(9, 10),  I16(10, 11),
#define _REJ_UNIFORM_MASK_01
    0,           1,           0,           1,           0,
    1,           0,           1,
#define _CBD2_MASK_E8_01
    I16(0, 1),   I16(0, 1),   I16(0, 1),   I16(0, 1),   I16(0, 1),
    I16(0, 1),   I16(0, 1),   I16(0, 1),
#define _CBD2_IDX8_LOW
    I16(0, 0),   I16(1, 1),   I16(2, 2),   I16(3, 3),   I16(4, 4),
    I16(5, 5),   I16(6, 6),   I16(7, 7),
#define _CBD2_IDX8_HIGH
    I16(8, 8),   I16(9, 9),   I16(10, 10), I16(11, 11), I16(12, 12),
    I16(13, 13), I16(14, 14), I16(15, 15),
#define _CBD3_MASK_E8_0122
    I16(0, 1),   I16(2, 2),   I16(3, 4),   I16(5, 5),   I16(6, 7),
    I16(8, 8),   I16(9, 10),  I16(11, 11),
#define _CBD3_IDX16_HIGH
    4,           5,           4,           5,           6,
    7,           6,           7,
#define _CBD3_MASK_E16_1100
    1,           1,           0,           0,           1,
    1,           0,           0,
#define _CBD3_IDX16_LOW
    0,           1,           0,           1,           2,
    3,           2,           3,
#define _ZETAS_EXP
    31498,       -758,
#define _ZETAS_EXP_1TO6_P0_L1
    14745,       -359,
#define _ZETAS_EXP_1TO6_P0_L2
    13525,       1493,        -12402,      1422,
#define _ZETAS_EXP_1TO6_P0_L3
    -20907,      -20907,      27758,       27758,       -3799,
    -3799,       -15690,      -15690,      -171,        -171,
    622,         622,         1577,        1577,        182,
    182,
#define _ZETAS_EXP_1TO6_P0_L4
    -5827,       17363,       -26360,      -29057,      5571,
    -1102,       21438,       -26242,      573,         -1325,
    264,         383,         -829,        1458,        -1602,
    -130,
#define _ZETAS_EXP_1TO6_P0_L5
    -5689,       1496,        -23565,      20710,       -12796,
    16064,       9134,        -25986,      1223,        -552,
    -1293,       -282,        516,         -320,        -1618,
    126,         -6516,       30967,       20179,       25080,
    26616,       -12442,      -650,        27837,       652,
    1015,        1491,        -1544,       -8,          -666,
    -1162,       1469,
#define _ZETAS_EXP_1TO6_P0_L6
    -1103,       -1251,       422,         -291,        -246,
    -777,        -1590,       418,         430,         871,
    587,         -460,        778,         1483,        644,
    329,         555,         1550,        177,         1574,
    1159,        -602,        -872,        -156,        843,
    105,         -235,        1653,        -147,        1119,
    349,         -75,
#define _ZETAS_EXP_1TO6_P1_L1
    787,         -1517,
#define _ZETAS_EXP_1TO6_P1_L2
    28191,       287,         -16694,      202,         0,
    0,
#define _ZETAS_EXP_1TO6_P1_L3
    10690,       10690,       1358,        1358,        -11202,
    -11202,      31164,       31164,       962,         962,
    -1202,       -1202,       -1474,       -1474,       1468,
    1468,
#define _ZETAS_EXP_1TO6_P1_L4
    -28073,      24313,       -10532,      8800,        18426,
    8859,        26675,       -16163,      -681,        1017,
    732,         608,         -1542,       411,         -205,
    -1571,
#define _ZETAS_EXP_1TO6_P1_L5
    19883,       -15887,      -28309,      -30199,      13426,
    -29156,      16832,       -24155,      -853,        -271,
    107,         -247,        -398,        -1508,       448,
    677,         -28250,      -8898,       9075,        18249,
    14017,       -12757,      4311,        -17915,      -90,
    830,         -1421,       -951,        961,         -725,
    -1065,       -1275,
#define _ZETAS_EXP_1TO6_P1_L6
    817,         1322,        -1215,       -874,        -1185,
    -1510,       -108,        958,         1097,        -1285,
    -136,        220,         -1530,       -854,        -308,
    -1460,       603,         -1465,       1218,        -1187,
    -1278,       -870,        996,         1522,        610,
    384,         -1335,       -1659,       794,         478,
    991,         1628,
#define _ZETAS_BASEMUL
    -1103,       -1251,       422,         -291,        -246,
    -777,        -1590,       418,         1103,        1251,
    -422,        291,         246,         777,         1590,
    -418,        430,         871,         587,         -460,
    778,         1483,        644,         329,         -430,
    -871,        -587,        460,         -778,        -1483,
    -644,        -329,        555,         1550,        177,
    1574,        1159,        -602,        -872,        -156,
    -555,        -1550,       -177,        -1574,       -1159,
    602,         872,         156,         843,         105,
    -235,        1653,        -147,        1119,        349,
    -75,         -843,        -105,        235,         -1653,
    147,         -1119,       -349,        75,          817,
    1322,        -1215,       -874,        -1185,       -1510,
    -108,        958,         -817,        -1322,       1215,
    874,         1185,        1510,        108,         -958,
    1097,        -1285,       -136,        220,         -1530,
    -854,        -308,        -1460,       -1097,       1285,
    136,         -220,        1530,        854,         308,
    1460,        603,         -1465,       1218,        -1187,
    -1278,       -870,        996,         1522,        -603,
    1465,        -1218,       1187,        1278,        870,
    -996,        -1522,       610,         384,         -1335,
    -1659,       794,         478,         991,         1628,
    -610,        -384,        1335,        1659,        -794,
    -478,        -991,        -1628,
#define _ZETA_EXP_INTT_0TO5_P0_L0
    -1628,       -991,        -478,        -794,        1659,
    1335,        -384,        -610,        -1522,       -996,
    870,         1278,        1187,        -1218,       1465,
    -603,        1460,        308,         854,         1530,
    -220,        136,         1285,        -1097,       -958,
    108,         1510,        1185,        874,         1215,
    -1322,       -817,
#define _ZETA_EXP_INTT_0TO5_P0_L1
    17915,       -4311,       12757,       -14017,      -18249,
    -9075,       8898,        28250,       1275,        1065,
    725,         -961,        951,         1421,        -830,
    90,          24155,       -16832,      29156,       -13426,
    30199,       28309,       15887,       -19883,      -677,
    -448,        1508,        398,         247,         -107,
    271,         853,
#define _ZETA_EXP_INTT_0TO5_P0_L2
    16163,       -26675,      -8859,       -18426,      -8800,
    10532,       -24313,      28073,       1571,        205,
    -411,        1542,        -608,        -732,        -1017,
    681,
#define _ZETA_EXP_INTT_0TO5_P0_L3
    -31164,      -31164,      11202,       11202,       -1358,
    -1358,       -10690,      -10690,      -1468,       -1468,
    1474,        1474,        1202,        1202,        -962,
    -962,
#define _ZETA_EXP_INTT_0TO5_P0_L4
    16694,       16694,       16694,       16694,       -28191,
    -28191,      -28191,      -28191,      -202,        -202,
    -202,        -202,        -287,        -287,        -287,
    -287,
#define _ZETA_EXP_INTT_0TO5_P0_L5
    -787,        1517,        0,           0,           0,
    0,           0,           0,
#define _ZETA_EXP_INTT_0TO5_P1_L0
    75,          -349,        -1119,       147,         -1653,
    235,         -105,        -843,        156,         872,
    602,         -1159,       -1574,       -177,        -1550,
    -555,        -329,        -644,        -1483,       -778,
    460,         -587,        -871,        -430,        -418,
    1590,        777,         246,         291,         -422,
    1251,        1103,
#define _ZETA_EXP_INTT_0TO5_P1_L1
    -27837,      650,         12442,       -26616,      -25080,
    -20179,      -30967,      6516,        -1469,       1162,
    666,         8,           1544,        -1491,       -1015,
    -652,        25986,       -9134,       -16064,      12796,
    -20710,      23565,       -1496,       5689,        -126,
    1618,        320,         -516,        282,         1293,
    552,         -1223,
#define _ZETA_EXP_INTT_0TO5_P1_L2
    26242,       -21438,      1102,        -5571,       29057,
    26360,       -17363,      5827,        130,         1602,
    -1458,       829,         -383,        -264,        1325,
    -573,
#define _ZETA_EXP_INTT_0TO5_P1_L3
    15690,       15690,       3799,        3799,        -27758,
    -27758,      20907,       20907,       -182,        -182,
    -1577,       -1577,       -622,        -622,        171,
    171,
#define _ZETA_EXP_INTT_0TO5_P1_L4
    12402,       12402,       12402,       12402,       -13525,
    -13525,      -13525,      -13525,      -1422,       -1422,
    -1422,       -1422,       -1493,       -1493,       -1493,
    -1493,
#define _ZETA_EXP_INTT_0TO5_P1_L5
    -14745,      359,
#define _ZETA_EXP_INTT_L6
    -31498,      758,
};

/* Plantard-domain twiddle factors for the scalar (RV64IM) Kyber inverse NTT.
 * Verbatim from PQRV `Kyber/RV64/ntt.c` (`zetas_intt_rv64im`): 3+4 layer
 * merging strategy with Gentleman-Sande butterflies. Using the real table
 * rather than a zero array keeps the kernels away from degenerate inputs.
 * The single-issue kernel reads the table with 64-bit loads (`ld`), so it is
 * over-aligned here. */
uint32_t zetas_intt_rv64im[128] __attribute__((aligned(16))) = {
    1515946703, 3153169142,  508325959, 1572714069, 3120914957, 2711931889,
    3235739856, 2967385035, 1877193577, 2731284400, 2073299023,  282546663,
    1103093133, 1748176836, 3151878974,  714752744, 1521107373, 2820305951,
    3458938817, 2384229368,  580575333, 2971255538, 3775029832, 2886104489,
     677337889,  918599194,  879894172, 1207596693,  948273044, 1113414472,
    3571183381, 1586905910, 1195985186, 2315850495,  576704831, 3318310570,
     717333078, 3883403894, 2923519343,  594767175, 2439706566, 3604727734,
    2172641913, 1598517417, 1006330578, 3811154519, 3711811629, 3253802200,
    1593356747,  140628248, 1666896290, 1330162597, 2251342125, 2443577068,
    1303069081, 2455188575, 3467969989, 3949202432, 1279846068, 3349274588,
    1824296713, 2591946320, 2575174144, 1214047530, 1150829327, 1317260922,
    3083500102, 4252391772,  598637677, 1404992306,  828287475, 2410032716,
    3066727926, 1912028097, 3479581496, 1052776604, 4222717922, 1986857806,
     696690400, 2164900908, 2097812203, 1744306334, 3456358482,  365117377,
     359956707,  405112566, 1513366368, 3319600737, 1643673276, 1357256112,
    2390680205, 3994358291,   61928036,  860541661, 2477121421, 1322421592,
    2750636911, 2567433139, 2875783149, 1343064271, 3362176262, 2182963252,
    2029433331,   98052723, 1137927653,  877313837, 3542799699,  732815087,
    4273034451, 3867921885, 3922108916, 3913077744, 2042335005, 3560862042,
      89021552,  700560902, 3266703874, 4045964987, 1819136044, 2924809511,
    3200905336, 1847519727, 3482161830,  886345009,   51606697,  966335388,
    1802363867, 2435836064,};

/* Plantard-domain twiddle factors for the scalar (RV64IM) Kyber *forward* NTT,
 * l=16 flavour (32-bit products): verbatim from PQRV `Kyber/RV64/ntt.c`
 * (`zetas_ntt_rv64im`), 4+3 layer merging with CT butterflies. Shared by the
 * single-issue and the dual-issue kernel, exactly as PQRV's `ntt()` does.
 * Read with 64-bit loads (packed pairs), hence the over-alignment. */
uint32_t zetas_ntt_rv64im[128] __attribute__((aligned(16))) = {
    2230699446u, 3328631909u, 4243360600u, 3408622288u,  812805467u, 2447447570u,
    1094061961u, 1370157786u, 2475831253u,  249002310u, 1028263423u, 3594406395u,
    4205945745u,  734105255u, 2252632292u,  381889553u,  372858381u,  427045412u,
      21932846u, 3562152210u,  752167598u, 3417653460u, 3157039644u, 4196914574u,
    2265533966u, 2112004045u,  932791035u, 2951903026u, 1419184148u, 1727534158u,
    1544330386u, 2972545705u, 1817845876u, 3434425636u, 4233039261u,  300609006u,
    1904287092u, 2937711185u, 2651294021u,  975366560u, 2781600929u, 3889854731u,
    3935010590u, 3929849920u,  838608815u, 2550660963u, 2197155094u, 2130066389u,
    3598276897u, 2308109491u,   72249375u, 3242190693u,  815385801u, 2382939200u,
    1228239371u, 1884934581u, 3466679822u, 2889974991u, 3696329620u,   42575525u,
    1211467195u, 2977706375u, 3144137970u, 3080919767u, 1719793153u, 1703020977u,
    2470670584u,  945692709u, 3015121229u,  345764865u,  826997308u, 1839778722u,
    2991898216u, 1851390229u, 2043625172u, 2964804700u, 2628071007u, 4154339049u,
    2701610550u, 1041165097u,  583155668u,  483812778u, 3288636719u, 2696449880u,
    2122325384u,  690239563u, 1855260731u, 3700200122u, 1371447954u,  411563403u,
    3577634219u,  976656727u, 3718262466u, 1979116802u, 3098982111u, 2708061387u,
     723783916u, 3181552825u, 3346694253u, 3087370604u, 3415073125u, 3376368103u,
    3617629408u, 1408862808u,  519937465u, 1323711759u, 3714391964u, 1910737929u,
     836028480u, 1474661346u, 2773859924u, 3580214553u, 1143088323u, 2546790461u,
    3191874164u, 4012420634u, 2221668274u, 1563682897u, 2417773720u, 1327582262u,
    1059227441u, 1583035408u, 1174052340u, 2722253228u, 3786641338u, 1141798155u,
    2779020594u,          0u,};

/* Same twiddles for the l=32 flavour (full 64-bit products). PQRV ships no
 * RV64 table for this variant, so it is derived from the l=16 one:
 *
 *   l=16 stores  b16 * q^-1 mod 2^32,  b16 = zeta * 2^32 mod+- q
 *   l=32 needs   b32 * q^-1 mod 2^64,  b32 = zeta * 2^64 mod+- q
 *                                          = b16 * 2^32   mod+- q
 *
 * so each entry is recovered (b16 = entry * q mod 2^32, verified to satisfy
 * |b16| <= q for all 128 entries), rescaled by 2^32 mod+- q, and re-multiplied
 * by q^-1 mod 2^64. The equivalence tests below only require that the naive and
 * the optimized kernel read the *same* table, but using a properly formed one
 * keeps the kernels off degenerate inputs. */
uint64_t zetas_ntt_l32_rv64im[128] __attribute__((aligned(16))) = {
     9580781166549658980ull, 14296365187795326876ull, 18225095001030554300ull,
    14639921250447772715ull,  3490972894694207725ull, 10511707271801447707ull,
     4698960340794743097ull,  5884782879627378737ull, 10633614261774896230ull,
     1069456775676162049ull,  4416357773129021519ull, 15437857912092163053ull,
    18064399423338281246ull,  3152958058858736819ull,  9674982022438232839ull,
     1640203137824580138ull,  1601414550105755607ull,  1834146076418702789ull,
       94200855888573859ull, 15299327241667789730ull,  3230535234296385879ull,
    14678709838166597246ull, 13559382021137660800ull, 18025610835619456716ull,
     9730394290607982168ull,  9070988299387965153ull,  4006306988672876485ull,
    12678326957238646470ull,  6095349498672426187ull,  7419702707929435150ull,
     6632848499918994678ull, 12766986586310245396ull,  7807588585117680453ull,
    14750745786787271373ull, 18180765186494754837ull,  1291105848355159365ull,
     8178850781855000957ull, 12617373462251922208ull, 11387221108883487104ull,
     4189167473633049271ull, 11946885017397955327ull, 16706798853179422686ull,
    16900741791773545338ull, 16878576884505645606ull,  3601797431033706383ull,
    10955005417159442338ull,  9436709269308310725ull,  9148565474825614214ull,
    15454481592543087851ull,  9913254775568154954ull,   310308701750596242ull,
    13925102991058006372ull,  3502055348328157591ull, 10234645930952701062ull,
     5275247929760136118ull,  8095732379600376963ull, 14889276457211644696ull,
    12412348070023849691ull, 15875614830633182752ull,   182860484960172786ull,
     5203211981139461991ull, 12789151493578145128ull, 13503969752967911471ull,
    13232449638936139759ull,  7386455347027585552ull,  7314419398406911425ull,
    10611449354506996499ull,  4061719256842625814ull, 12949847071270418182ull,
     1485048786949282017ull,  3551926389680931987ull,  7901789441006254312ull,
    12850104988564869389ull,  7951660482359028708ull,  8777303278088293710ull,
    12733739225408395799ull, 11287479026177938312ull, 17842750350659283930ull,
    11603328954745509488ull,  4471770041298770848ull,  2504634521272669670ull,
     2077960056365599837ull, 14124587156469103956ull, 11581164047477609756ull,
     9115318113923764616ull,  2964556347081589100ull,  7968284162809953507ull,
    15892238511084107550ull,  5890324106444353670ull,  1767651354615003594ull,
    15365821963471488925ull,  4194708700450024203ull, 15969815686521756611ull,
     8500241937239547065ull, 13310026814373788820ull, 11631035088830384152ull,
     3108628244322937356ull, 13664665330660184525ull, 14373942363232975936ull,
    13260155773021014424ull, 14667627384532647380ull, 14501390580023399393ull,
    15537599994797711845ull,  6051019684136626724ull,  2233114407240897958ull,
     5685298714216281153ull, 15953192006070831812ull,  8206556915939875621ull,
     3590714977399756518ull,  6333622251802348302ull, 11913637656496105730ull,
    15376904417105438791ull,  4909526959839790547ull, 10938381736708517540ull,
    13708995145195983989ull, 17233215400792041311ull,  9541992578830834450ull,
     6715966902173618672ull, 10384259055011024250ull,  5701922394667205952ull,
     4549347216736419909ull,  6799085304428242665ull,  5042516403447188937ull,
    11691988583817108414ull, 16263500707821428054ull,  4903985733022815614ull,
    11935802563764005461ull,                    0ull,};

/* ===== RVV VLEN=128: forward / inverse NTT =====
 * Signature: void f(int16_t r[256], const int16_t qdata[]);
 * a0: coefficients (in-place), a1: the `zetas` table above (PQRV
 * `qdata_vlen128`), from which the kernels index twiddles and shuffle masks
 * at the offsets defined in KYBER_NTT_RVV_VLEN128_CONSTS_H. */
extern void ntt_rvv_vlen128(int16_t *r, const int16_t *zetas);
extern void ntt_rvv_vlen128_opt_c908(int16_t *r, const int16_t *zetas);
extern void intt_rvv_vlen128(int16_t *r, const int16_t *zetas);
extern void intt_rvv_vlen128_opt_c908(int16_t *r, const int16_t *zetas);

/* ===== RVV VLEN=128: NTT <-> normal coefficient order =====
 * The RVV NTT leaves the coefficients in a permuted (vector-friendly) order;
 * these kernels convert between that order and the natural one. */
extern void normal2ntt_order_rvv_vlen128(int16_t *r, const int16_t *zetas);
extern void ntt2normal_order_rvv_vlen128(int16_t *r, const int16_t *zetas);
extern void normal2ntt_order_rvv_vlen128_opt_c908(int16_t *r, const int16_t *zetas);
extern void ntt2normal_order_rvv_vlen128_opt_c908(int16_t *r, const int16_t *zetas);

/* ===== RV64IM: scalar inverse NTT (Plantard arithmetic) =====
 * Signature: void f(int16_t r[256], uint32_t zetas[128]);
 * The single-issue and dual-issue kernels are independent implementations, so
 * each optimized variant is only ever compared against its own naive kernel. */
extern void intt_rv64im(int16_t *r, uint32_t *zetas);
extern void intt_rv64im_opt_c908(int16_t *r, uint32_t *zetas);
extern void intt_dual_rv64im(int16_t *r, uint32_t *zetas);
extern void intt_dual_rv64im_opt_c908(int16_t *r, uint32_t *zetas);

/* ===== RV64IM: scalar forward NTT (Plantard arithmetic) =====
 * Single-issue and dual-issue are l=16 kernels (32-bit products) and take
 * uint32_t twiddles; the l32 kernel does full 64-bit products and takes
 * uint64_t twiddles. All three are independent implementations, so each
 * optimized variant is only ever compared against its own naive kernel. */
extern void ntt_rv64im(int16_t *r, uint32_t *zetas);
extern void ntt_rv64im_opt_c908(int16_t *r, uint32_t *zetas);
extern void ntt_dual_rv64im(int16_t *r, uint32_t *zetas);
extern void ntt_dual_rv64im_opt_c908(int16_t *r, uint32_t *zetas);
extern void ntt_dual_l32_rv64im(int16_t *r, uint64_t *zetas);
extern void ntt_dual_l32_rv64im_opt_c908(int16_t *r, uint64_t *zetas);

/* ===== Wrapper functions that bind the table argument ===== */

/* RV64IM scalar forward NTT */
void ntt_kyber_rv64im_wrap(int16_t *r) { ntt_rv64im(r, zetas_ntt_rv64im); }
void ntt_kyber_rv64im_opt_c908_wrap(int16_t *r) { ntt_rv64im_opt_c908(r, zetas_ntt_rv64im); }
void ntt_kyber_dual_rv64im_wrap(int16_t *r) { ntt_dual_rv64im(r, zetas_ntt_rv64im); }
void ntt_kyber_dual_rv64im_opt_c908_wrap(int16_t *r) { ntt_dual_rv64im_opt_c908(r, zetas_ntt_rv64im); }
void ntt_kyber_dual_l32_rv64im_wrap(int16_t *r) { ntt_dual_l32_rv64im(r, zetas_ntt_l32_rv64im); }
void ntt_kyber_dual_l32_rv64im_opt_c908_wrap(int16_t *r) { ntt_dual_l32_rv64im_opt_c908(r, zetas_ntt_l32_rv64im); }

/* RV64IM scalar INTT */
void intt_kyber_rv64im_wrap(int16_t *r) { intt_rv64im(r, zetas_intt_rv64im); }
void intt_kyber_rv64im_opt_c908_wrap(int16_t *r) { intt_rv64im_opt_c908(r, zetas_intt_rv64im); }
void intt_kyber_dual_rv64im_wrap(int16_t *r) { intt_dual_rv64im(r, zetas_intt_rv64im); }
void intt_kyber_dual_rv64im_opt_c908_wrap(int16_t *r) { intt_dual_rv64im_opt_c908(r, zetas_intt_rv64im); }

#ifdef VECTOR128
/* RVV forward NTT */
void ntt_kyber_rvv_vlen128_wrap(int16_t *r) { ntt_rvv_vlen128(r, zetas); }
void ntt_kyber_rvv_vlen128_opt_c908_wrap(int16_t *r) { ntt_rvv_vlen128_opt_c908(r, zetas); }

/* RVV inverse NTT */
void intt_kyber_rvv_vlen128_wrap(int16_t *r) { intt_rvv_vlen128(r, zetas); }
void intt_kyber_rvv_vlen128_opt_c908_wrap(int16_t *r) { intt_rvv_vlen128_opt_c908(r, zetas); }

/* RVV order conversion */
void kyber_normal2ntt_order_rvv_vlen128_wrap(int16_t *r) { normal2ntt_order_rvv_vlen128(r, zetas); }
void kyber_ntt2normal_order_rvv_vlen128_wrap(int16_t *r) { ntt2normal_order_rvv_vlen128(r, zetas); }
void kyber_normal2ntt_order_rvv_vlen128_opt_c908_wrap(int16_t *r) { normal2ntt_order_rvv_vlen128_opt_c908(r, zetas); }
void kyber_ntt2normal_order_rvv_vlen128_opt_c908_wrap(int16_t *r) { ntt2normal_order_rvv_vlen128_opt_c908(r, zetas); }
#endif /* VECTOR128 */

#endif

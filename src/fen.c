
// fen.c

// includes

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "board.h"
#include "colour.h"
#include "fen.h"
#include "option.h"
#include "piece.h"
#include "square.h"
#include "util.h"

// "constants"

// const char * StartFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w HAha - 0 1";
const char * StartFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const char * StartFen960[960] = {
"qrbbnnkr/pppppppp/8/8/8/8/PPPPPPPP/QRBBNNKR w BHbh - 0 1",
"nqrkrbbn/pppppppp/8/8/8/8/PPPPPPPP/NQRKRBBN w CEce - 0 1",
"rqnkbrnb/pppppppp/8/8/8/8/PPPPPPPP/RQNKBRNB w AFaf - 0 1",
"rnqkbbrn/pppppppp/8/8/8/8/PPPPPPPP/RNQKBBRN w AGag - 0 1",
"rnnkbbqr/pppppppp/8/8/8/8/PPPPPPPP/RNNKBBQR w AHah - 0 1",
"nrbqnbkr/pppppppp/8/8/8/8/PPPPPPPP/NRBQNBKR w BHbh - 0 1",
"rnbkqbrn/pppppppp/8/8/8/8/PPPPPPPP/RNBKQBRN w AGag - 0 1",
"nbrkbnrq/pppppppp/8/8/8/8/PPPPPPPP/NBRKBNRQ w CGcg - 0 1",
"rnqbnkbr/pppppppp/8/8/8/8/PPPPPPPP/RNQBNKBR w AHah - 0 1",
"nnrbkqbr/pppppppp/8/8/8/8/PPPPPPPP/NNRBKQBR w CHch - 0 1",
"nbrkbqnr/pppppppp/8/8/8/8/PPPPPPPP/NBRKBQNR w CHch - 0 1",
"nrbknrqb/pppppppp/8/8/8/8/PPPPPPPP/NRBKNRQB w BFbf - 0 1",
"rkbbqnrn/pppppppp/8/8/8/8/PPPPPPPP/RKBBQNRN w AGag - 0 1",
"rbbnkrqn/pppppppp/8/8/8/8/PPPPPPPP/RBBNKRQN w AFaf - 0 1",
"nrnkqrbb/pppppppp/8/8/8/8/PPPPPPPP/NRNKQRBB w BFbf - 0 1",
"qnrnbkrb/pppppppp/8/8/8/8/PPPPPPPP/QNRNBKRB w CGcg - 0 1",
"brqbknnr/pppppppp/8/8/8/8/PPPPPPPP/BRQBKNNR w BHbh - 0 1",
"rnkrbqnb/pppppppp/8/8/8/8/PPPPPPPP/RNKRBQNB w ADad - 0 1",
"brknrbqn/pppppppp/8/8/8/8/PPPPPPPP/BRKNRBQN w BEbe - 0 1",
"bqnrkbrn/pppppppp/8/8/8/8/PPPPPPPP/BQNRKBRN w DGdg - 0 1",
"bbnqrnkr/pppppppp/8/8/8/8/PPPPPPPP/BBNQRNKR w EHeh - 0 1",
"qnbbnrkr/pppppppp/8/8/8/8/PPPPPPPP/QNBBNRKR w FHfh - 0 1",
"rnkrbbqn/pppppppp/8/8/8/8/PPPPPPPP/RNKRBBQN w ADad - 0 1",
"rbkrqnbn/pppppppp/8/8/8/8/PPPPPPPP/RBKRQNBN w ADad - 0 1",
"rbknrnbq/pppppppp/8/8/8/8/PPPPPPPP/RBKNRNBQ w AEae - 0 1",
"rqbnkbnr/pppppppp/8/8/8/8/PPPPPPPP/RQBNKBNR w AHah - 0 1",
"nnrkbbrq/pppppppp/8/8/8/8/PPPPPPPP/NNRKBBRQ w CGcg - 0 1",
"bnnbrkqr/pppppppp/8/8/8/8/PPPPPPPP/BNNBRKQR w EHeh - 0 1",
"bnnbqrkr/pppppppp/8/8/8/8/PPPPPPPP/BNNBQRKR w FHfh - 0 1",
"nrqnkrbb/pppppppp/8/8/8/8/PPPPPPPP/NRQNKRBB w BFbf - 0 1",
"bnnrkbqr/pppppppp/8/8/8/8/PPPPPPPP/BNNRKBQR w DHdh - 0 1",
"nrkqbnrb/pppppppp/8/8/8/8/PPPPPPPP/NRKQBNRB w BGbg - 0 1",
"nnbbrkrq/pppppppp/8/8/8/8/PPPPPPPP/NNBBRKRQ w EGeg - 0 1",
"qnrbkrbn/pppppppp/8/8/8/8/PPPPPPPP/QNRBKRBN w CFcf - 0 1",
"nnbrkrqb/pppppppp/8/8/8/8/PPPPPPPP/NNBRKRQB w DFdf - 0 1",
"brnqnbkr/pppppppp/8/8/8/8/PPPPPPPP/BRNQNBKR w BHbh - 0 1",
"nnqbbrkr/pppppppp/8/8/8/8/PPPPPPPP/NNQBBRKR w FHfh - 0 1",
"rnbqkrnb/pppppppp/8/8/8/8/PPPPPPPP/RNBQKRNB w AFaf - 0 1",
"qnrkbbnr/pppppppp/8/8/8/8/PPPPPPPP/QNRKBBNR w CHch - 0 1",
"nrknrbbq/pppppppp/8/8/8/8/PPPPPPPP/NRKNRBBQ w BEbe - 0 1",
"bqnbrnkr/pppppppp/8/8/8/8/PPPPPPPP/BQNBRNKR w EHeh - 0 1",
"rkqbbnnr/pppppppp/8/8/8/8/PPPPPPPP/RKQBBNNR w AHah - 0 1",
"qnbnrkrb/pppppppp/8/8/8/8/PPPPPPPP/QNBNRKRB w EGeg - 0 1",
"qrkbbnrn/pppppppp/8/8/8/8/PPPPPPPP/QRKBBNRN w BGbg - 0 1",
"nrnkrqbb/pppppppp/8/8/8/8/PPPPPPPP/NRNKRQBB w BEbe - 0 1",
"rqnkrbbn/pppppppp/8/8/8/8/PPPPPPPP/RQNKRBBN w AEae - 0 1",
"rnnbbkqr/pppppppp/8/8/8/8/PPPPPPPP/RNNBBKQR w AHah - 0 1",
"nqbrkrnb/pppppppp/8/8/8/8/PPPPPPPP/NQBRKRNB w DFdf - 0 1",
"qnrnbbkr/pppppppp/8/8/8/8/PPPPPPPP/QNRNBBKR w CHch - 0 1",
"nrnqbbkr/pppppppp/8/8/8/8/PPPPPPPP/NRNQBBKR w BHbh - 0 1",
"qrknnrbb/pppppppp/8/8/8/8/PPPPPPPP/QRKNNRBB w BFbf - 0 1",
"rnnkqbbr/pppppppp/8/8/8/8/PPPPPPPP/RNNKQBBR w AHah - 0 1",
"nrknbrqb/pppppppp/8/8/8/8/PPPPPPPP/NRKNBRQB w BFbf - 0 1",
"rbkqbrnn/pppppppp/8/8/8/8/PPPPPPPP/RBKQBRNN w AFaf - 0 1",
"nqbbnrkr/pppppppp/8/8/8/8/PPPPPPPP/NQBBNRKR w FHfh - 0 1",
"rknbbrnq/pppppppp/8/8/8/8/PPPPPPPP/RKNBBRNQ w AFaf - 0 1",
"rnqbknbr/pppppppp/8/8/8/8/PPPPPPPP/RNQBKNBR w AHah - 0 1",
"nbrnbkrq/pppppppp/8/8/8/8/PPPPPPPP/NBRNBKRQ w CGcg - 0 1",
"nrbbkrqn/pppppppp/8/8/8/8/PPPPPPPP/NRBBKRQN w BFbf - 0 1",
"nrbkqrnb/pppppppp/8/8/8/8/PPPPPPPP/NRBKQRNB w BFbf - 0 1",
"brknnbrq/pppppppp/8/8/8/8/PPPPPPPP/BRKNNBRQ w BGbg - 0 1",
"qbbrnnkr/pppppppp/8/8/8/8/PPPPPPPP/QBBRNNKR w DHdh - 0 1",
"rnkbrqbn/pppppppp/8/8/8/8/PPPPPPPP/RNKBRQBN w AEae - 0 1",
"nrkrbqnb/pppppppp/8/8/8/8/PPPPPPPP/NRKRBQNB w BDbd - 0 1",
"rnkqbbrn/pppppppp/8/8/8/8/PPPPPPPP/RNKQBBRN w AGag - 0 1",
"nqrbnkbr/pppppppp/8/8/8/8/PPPPPPPP/NQRBNKBR w CHch - 0 1",
"bbnrkrnq/pppppppp/8/8/8/8/PPPPPPPP/BBNRKRNQ w DFdf - 0 1",
"rbknbrnq/pppppppp/8/8/8/8/PPPPPPPP/RBKNBRNQ w AFaf - 0 1",
"qrnbbknr/pppppppp/8/8/8/8/PPPPPPPP/QRNBBKNR w BHbh - 0 1",
"rqknnrbb/pppppppp/8/8/8/8/PPPPPPPP/RQKNNRBB w AFaf - 0 1",
"rbqknrbn/pppppppp/8/8/8/8/PPPPPPPP/RBQKNRBN w AFaf - 0 1",
"nrkqbbrn/pppppppp/8/8/8/8/PPPPPPPP/NRKQBBRN w BGbg - 0 1",
"nqnrbbkr/pppppppp/8/8/8/8/PPPPPPPP/NQNRBBKR w DHdh - 0 1",
"brnknrqb/pppppppp/8/8/8/8/PPPPPPPP/BRNKNRQB w BFbf - 0 1",
"bqrknbrn/pppppppp/8/8/8/8/PPPPPPPP/BQRKNBRN w CGcg - 0 1",
"bbqrkrnn/pppppppp/8/8/8/8/PPPPPPPP/BBQRKRNN w DFdf - 0 1",
"bbrnnkrq/pppppppp/8/8/8/8/PPPPPPPP/BBRNNKRQ w CGcg - 0 1",
"qnnbbrkr/pppppppp/8/8/8/8/PPPPPPPP/QNNBBRKR w FHfh - 0 1",
"qrbknbrn/pppppppp/8/8/8/8/PPPPPPPP/QRBKNBRN w BGbg - 0 1",
"rkrbbqnn/pppppppp/8/8/8/8/PPPPPPPP/RKRBBQNN w ACac - 0 1",
"nrbkrnqb/pppppppp/8/8/8/8/PPPPPPPP/NRBKRNQB w BEbe - 0 1",
"nnbbrqkr/pppppppp/8/8/8/8/PPPPPPPP/NNBBRQKR w EHeh - 0 1",
"rbbnnkqr/pppppppp/8/8/8/8/PPPPPPPP/RBBNNKQR w AHah - 0 1",
"rqknbbnr/pppppppp/8/8/8/8/PPPPPPPP/RQKNBBNR w AHah - 0 1",
"brknqrnb/pppppppp/8/8/8/8/PPPPPPPP/BRKNQRNB w BFbf - 0 1",
"qrknbrnb/pppppppp/8/8/8/8/PPPPPPPP/QRKNBRNB w BFbf - 0 1",
"brqknbrn/pppppppp/8/8/8/8/PPPPPPPP/BRQKNBRN w BGbg - 0 1",
"rkbbnnqr/pppppppp/8/8/8/8/PPPPPPPP/RKBBNNQR w AHah - 0 1",
"bnrkqrnb/pppppppp/8/8/8/8/PPPPPPPP/BNRKQRNB w CFcf - 0 1",
"rbnkbrnq/pppppppp/8/8/8/8/PPPPPPPP/RBNKBRNQ w AFaf - 0 1",
"rnnqkbbr/pppppppp/8/8/8/8/PPPPPPPP/RNNQKBBR w AHah - 0 1",
"rknqrnbb/pppppppp/8/8/8/8/PPPPPPPP/RKNQRNBB w AEae - 0 1",
"rnknbqrb/pppppppp/8/8/8/8/PPPPPPPP/RNKNBQRB w AGag - 0 1",
"bbrkqnrn/pppppppp/8/8/8/8/PPPPPPPP/BBRKQNRN w CGcg - 0 1",
"bbnrknqr/pppppppp/8/8/8/8/PPPPPPPP/BBNRKNQR w DHdh - 0 1",
"rknqbbrn/pppppppp/8/8/8/8/PPPPPPPP/RKNQBBRN w AGag - 0 1",
"rkbbrqnn/pppppppp/8/8/8/8/PPPPPPPP/RKBBRQNN w AEae - 0 1",
"rqnbknbr/pppppppp/8/8/8/8/PPPPPPPP/RQNBKNBR w AHah - 0 1",
"rqknbnrb/pppppppp/8/8/8/8/PPPPPPPP/RQKNBNRB w AGag - 0 1",
"rbbkqnrn/pppppppp/8/8/8/8/PPPPPPPP/RBBKQNRN w AGag - 0 1",
"rbkrbqnn/pppppppp/8/8/8/8/PPPPPPPP/RBKRBQNN w ADad - 0 1",
"bqrnkrnb/pppppppp/8/8/8/8/PPPPPPPP/BQRNKRNB w CFcf - 0 1",
"rnknbbrq/pppppppp/8/8/8/8/PPPPPPPP/RNKNBBRQ w AGag - 0 1",
"nnqrbkrb/pppppppp/8/8/8/8/PPPPPPPP/NNQRBKRB w DGdg - 0 1",
"rnbqnkrb/pppppppp/8/8/8/8/PPPPPPPP/RNBQNKRB w AGag - 0 1",
"nrkrnqbb/pppppppp/8/8/8/8/PPPPPPPP/NRKRNQBB w BDbd - 0 1",
"rnbkqnrb/pppppppp/8/8/8/8/PPPPPPPP/RNBKQNRB w AGag - 0 1",
"qrnkbnrb/pppppppp/8/8/8/8/PPPPPPPP/QRNKBNRB w BGbg - 0 1",
"rnnkbqrb/pppppppp/8/8/8/8/PPPPPPPP/RNNKBQRB w AGag - 0 1",
"nrqkbbrn/pppppppp/8/8/8/8/PPPPPPPP/NRQKBBRN w BGbg - 0 1",
"bbrkqrnn/pppppppp/8/8/8/8/PPPPPPPP/BBRKQRNN w CFcf - 0 1",
"brqbknrn/pppppppp/8/8/8/8/PPPPPPPP/BRQBKNRN w BGbg - 0 1",
"bbrnknqr/pppppppp/8/8/8/8/PPPPPPPP/BBRNKNQR w CHch - 0 1",
"bnrqknrb/pppppppp/8/8/8/8/PPPPPPPP/BNRQKNRB w CGcg - 0 1",
"bbrqnkrn/pppppppp/8/8/8/8/PPPPPPPP/BBRQNKRN w CGcg - 0 1",
"rnknrqbb/pppppppp/8/8/8/8/PPPPPPPP/RNKNRQBB w AEae - 0 1",
"nrkbrnbq/pppppppp/8/8/8/8/PPPPPPPP/NRKBRNBQ w BEbe - 0 1",
"nqnbrkbr/pppppppp/8/8/8/8/PPPPPPPP/NQNBRKBR w EHeh - 0 1",
"rbbnkrnq/pppppppp/8/8/8/8/PPPPPPPP/RBBNKRNQ w AFaf - 0 1",
"bbrnkrqn/pppppppp/8/8/8/8/PPPPPPPP/BBRNKRQN w CFcf - 0 1",
"rnbknqrb/pppppppp/8/8/8/8/PPPPPPPP/RNBKNQRB w AGag - 0 1",
"nbqrkrbn/pppppppp/8/8/8/8/PPPPPPPP/NBQRKRBN w DFdf - 0 1",
"rkbrnnqb/pppppppp/8/8/8/8/PPPPPPPP/RKBRNNQB w ADad - 0 1",
"nqrnbkrb/pppppppp/8/8/8/8/PPPPPPPP/NQRNBKRB w CGcg - 0 1",
"nrnkbbrq/pppppppp/8/8/8/8/PPPPPPPP/NRNKBBRQ w BGbg - 0 1",
"rnbbkqnr/pppppppp/8/8/8/8/PPPPPPPP/RNBBKQNR w AHah - 0 1",
"nbqnrkbr/pppppppp/8/8/8/8/PPPPPPPP/NBQNRKBR w EHeh - 0 1",
"qrbkrnnb/pppppppp/8/8/8/8/PPPPPPPP/QRBKRNNB w BEbe - 0 1",
"qbnrkrbn/pppppppp/8/8/8/8/PPPPPPPP/QBNRKRBN w DFdf - 0 1",
"rnkbnrbq/pppppppp/8/8/8/8/PPPPPPPP/RNKBNRBQ w AFaf - 0 1",
"rknrnbbq/pppppppp/8/8/8/8/PPPPPPPP/RKNRNBBQ w ADad - 0 1",
"qbbrknnr/pppppppp/8/8/8/8/PPPPPPPP/QBBRKNNR w DHdh - 0 1",
"rkbbrnnq/pppppppp/8/8/8/8/PPPPPPPP/RKBBRNNQ w AEae - 0 1",
"rnkbqnbr/pppppppp/8/8/8/8/PPPPPPPP/RNKBQNBR w AHah - 0 1",
"rnbbnkrq/pppppppp/8/8/8/8/PPPPPPPP/RNBBNKRQ w AGag - 0 1",
"nrbqkbrn/pppppppp/8/8/8/8/PPPPPPPP/NRBQKBRN w BGbg - 0 1",
"bbrnkrnq/pppppppp/8/8/8/8/PPPPPPPP/BBRNKRNQ w CFcf - 0 1",
"nrbqnkrb/pppppppp/8/8/8/8/PPPPPPPP/NRBQNKRB w BGbg - 0 1",
"rkbnrqnb/pppppppp/8/8/8/8/PPPPPPPP/RKBNRQNB w AEae - 0 1",
"bbqrknnr/pppppppp/8/8/8/8/PPPPPPPP/BBQRKNNR w DHdh - 0 1",
"rnbnqbkr/pppppppp/8/8/8/8/PPPPPPPP/RNBNQBKR w AHah - 0 1",
"nbrnbqkr/pppppppp/8/8/8/8/PPPPPPPP/NBRNBQKR w CHch - 0 1",
"nrkqnrbb/pppppppp/8/8/8/8/PPPPPPPP/NRKQNRBB w BFbf - 0 1",
"qrbnkbrn/pppppppp/8/8/8/8/PPPPPPPP/QRBNKBRN w BGbg - 0 1",
"qbrkbrnn/pppppppp/8/8/8/8/PPPPPPPP/QBRKBRNN w CFcf - 0 1",
"bnrkrnqb/pppppppp/8/8/8/8/PPPPPPPP/BNRKRNQB w CEce - 0 1",
"nrbkqbrn/pppppppp/8/8/8/8/PPPPPPPP/NRBKQBRN w BGbg - 0 1",
"qbbnnrkr/pppppppp/8/8/8/8/PPPPPPPP/QBBNNRKR w FHfh - 0 1",
"rqnbnkbr/pppppppp/8/8/8/8/PPPPPPPP/RQNBNKBR w AHah - 0 1",
"qnrknrbb/pppppppp/8/8/8/8/PPPPPPPP/QNRKNRBB w CFcf - 0 1",
"qrbbknrn/pppppppp/8/8/8/8/PPPPPPPP/QRBBKNRN w BGbg - 0 1",
"rkbbrnqn/pppppppp/8/8/8/8/PPPPPPPP/RKBBRNQN w AEae - 0 1",
"brkqrbnn/pppppppp/8/8/8/8/PPPPPPPP/BRKQRBNN w BEbe - 0 1",
"brnkrbnq/pppppppp/8/8/8/8/PPPPPPPP/BRNKRBNQ w BEbe - 0 1",
"qbrkbnrn/pppppppp/8/8/8/8/PPPPPPPP/QBRKBNRN w CGcg - 0 1",
"nbbqrknr/pppppppp/8/8/8/8/PPPPPPPP/NBBQRKNR w EHeh - 0 1",
"bbqrnnkr/pppppppp/8/8/8/8/PPPPPPPP/BBQRNNKR w DHdh - 0 1",
"qnrnkbbr/pppppppp/8/8/8/8/PPPPPPPP/QNRNKBBR w CHch - 0 1",
"rnkqbbnr/pppppppp/8/8/8/8/PPPPPPPP/RNKQBBNR w AHah - 0 1",
"rnnbqkbr/pppppppp/8/8/8/8/PPPPPPPP/RNNBQKBR w AHah - 0 1",
"bbnrkrqn/pppppppp/8/8/8/8/PPPPPPPP/BBNRKRQN w DFdf - 0 1",
"nrbqknrb/pppppppp/8/8/8/8/PPPPPPPP/NRBQKNRB w BGbg - 0 1",
"rnbbkrqn/pppppppp/8/8/8/8/PPPPPPPP/RNBBKRQN w AFaf - 0 1",
"rbknbnrq/pppppppp/8/8/8/8/PPPPPPPP/RBKNBNRQ w AGag - 0 1",
"rnqbbkrn/pppppppp/8/8/8/8/PPPPPPPP/RNQBBKRN w AGag - 0 1",
"rnqbkrbn/pppppppp/8/8/8/8/PPPPPPPP/RNQBKRBN w AFaf - 0 1",
"rknqbbnr/pppppppp/8/8/8/8/PPPPPPPP/RKNQBBNR w AHah - 0 1",
"rqkbbnrn/pppppppp/8/8/8/8/PPPPPPPP/RQKBBNRN w AGag - 0 1",
"nqrkbbrn/pppppppp/8/8/8/8/PPPPPPPP/NQRKBBRN w CGcg - 0 1",
"qrkbnrbn/pppppppp/8/8/8/8/PPPPPPPP/QRKBNRBN w BFbf - 0 1",
"qnbnrbkr/pppppppp/8/8/8/8/PPPPPPPP/QNBNRBKR w EHeh - 0 1",
"brknrnqb/pppppppp/8/8/8/8/PPPPPPPP/BRKNRNQB w BEbe - 0 1",
"bbrqnknr/pppppppp/8/8/8/8/PPPPPPPP/BBRQNKNR w CHch - 0 1",
"rkqbrnbn/pppppppp/8/8/8/8/PPPPPPPP/RKQBRNBN w AEae - 0 1",
"rnqnbbkr/pppppppp/8/8/8/8/PPPPPPPP/RNQNBBKR w AHah - 0 1",
"nqrkbrnb/pppppppp/8/8/8/8/PPPPPPPP/NQRKBRNB w CFcf - 0 1",
"rbnnbkrq/pppppppp/8/8/8/8/PPPPPPPP/RBNNBKRQ w AGag - 0 1",
"qnbrkrnb/pppppppp/8/8/8/8/PPPPPPPP/QNBRKRNB w DFdf - 0 1",
"nqbrnbkr/pppppppp/8/8/8/8/PPPPPPPP/NQBRNBKR w DHdh - 0 1",
"nrnbqkbr/pppppppp/8/8/8/8/PPPPPPPP/NRNBQKBR w BHbh - 0 1",
"bnrbqkrn/pppppppp/8/8/8/8/PPPPPPPP/BNRBQKRN w CGcg - 0 1",
"nnbqrbkr/pppppppp/8/8/8/8/PPPPPPPP/NNBQRBKR w EHeh - 0 1",
"rqnbkrbn/pppppppp/8/8/8/8/PPPPPPPP/RQNBKRBN w AFaf - 0 1",
"qnbrknrb/pppppppp/8/8/8/8/PPPPPPPP/QNBRKNRB w DGdg - 0 1",
"rknnbrqb/pppppppp/8/8/8/8/PPPPPPPP/RKNNBRQB w AFaf - 0 1",
"qrnnbbkr/pppppppp/8/8/8/8/PPPPPPPP/QRNNBBKR w BHbh - 0 1",
"qrbnknrb/pppppppp/8/8/8/8/PPPPPPPP/QRBNKNRB w BGbg - 0 1",
"rkbbnqnr/pppppppp/8/8/8/8/PPPPPPPP/RKBBNQNR w AHah - 0 1",
"rqkrnbbn/pppppppp/8/8/8/8/PPPPPPPP/RQKRNBBN w ADad - 0 1",
"bqnbnrkr/pppppppp/8/8/8/8/PPPPPPPP/BQNBNRKR w FHfh - 0 1",
"rqbbnnkr/pppppppp/8/8/8/8/PPPPPPPP/RQBBNNKR w AHah - 0 1",
"bnnqrkrb/pppppppp/8/8/8/8/PPPPPPPP/BNNQRKRB w EGeg - 0 1",
"rbnkbqrn/pppppppp/8/8/8/8/PPPPPPPP/RBNKBQRN w AGag - 0 1",
"rbknbqrn/pppppppp/8/8/8/8/PPPPPPPP/RBKNBQRN w AGag - 0 1",
"qnrkrbbn/pppppppp/8/8/8/8/PPPPPPPP/QNRKRBBN w CEce - 0 1",
"qbrknnbr/pppppppp/8/8/8/8/PPPPPPPP/QBRKNNBR w CHch - 0 1",
"nrbbqnkr/pppppppp/8/8/8/8/PPPPPPPP/NRBBQNKR w BHbh - 0 1",
"bnqrkbnr/pppppppp/8/8/8/8/PPPPPPPP/BNQRKBNR w DHdh - 0 1",
"nrqbbknr/pppppppp/8/8/8/8/PPPPPPPP/NRQBBKNR w BHbh - 0 1",
"qnrkrnbb/pppppppp/8/8/8/8/PPPPPPPP/QNRKRNBB w CEce - 0 1",
"rnbknrqb/pppppppp/8/8/8/8/PPPPPPPP/RNBKNRQB w AFaf - 0 1",
"rbnkqrbn/pppppppp/8/8/8/8/PPPPPPPP/RBNKQRBN w AFaf - 0 1",
"bnrnqbkr/pppppppp/8/8/8/8/PPPPPPPP/BNRNQBKR w CHch - 0 1",
"qbrkrnbn/pppppppp/8/8/8/8/PPPPPPPP/QBRKRNBN w CEce - 0 1",
"brnknqrb/pppppppp/8/8/8/8/PPPPPPPP/BRNKNQRB w BGbg - 0 1",
"brnnkbqr/pppppppp/8/8/8/8/PPPPPPPP/BRNNKBQR w BHbh - 0 1",
"nnbrkqrb/pppppppp/8/8/8/8/PPPPPPPP/NNBRKQRB w DGdg - 0 1",
"nnrbbkqr/pppppppp/8/8/8/8/PPPPPPPP/NNRBBKQR w CHch - 0 1",
"brkbrnnq/pppppppp/8/8/8/8/PPPPPPPP/BRKBRNNQ w BEbe - 0 1",
"qnnrbkrb/pppppppp/8/8/8/8/PPPPPPPP/QNNRBKRB w DGdg - 0 1",
"nnrkbrqb/pppppppp/8/8/8/8/PPPPPPPP/NNRKBRQB w CFcf - 0 1",
"rnkbnqbr/pppppppp/8/8/8/8/PPPPPPPP/RNKBNQBR w AHah - 0 1",
"qrkbrnbn/pppppppp/8/8/8/8/PPPPPPPP/QRKBRNBN w BEbe - 0 1",
"bqnnrkrb/pppppppp/8/8/8/8/PPPPPPPP/BQNNRKRB w EGeg - 0 1",
"rnbbkqrn/pppppppp/8/8/8/8/PPPPPPPP/RNBBKQRN w AGag - 0 1",
"nnrqkbbr/pppppppp/8/8/8/8/PPPPPPPP/NNRQKBBR w CHch - 0 1",
"rkqnrnbb/pppppppp/8/8/8/8/PPPPPPPP/RKQNRNBB w AEae - 0 1",
"rbbkrnnq/pppppppp/8/8/8/8/PPPPPPPP/RBBKRNNQ w AEae - 0 1",
"brkrnbnq/pppppppp/8/8/8/8/PPPPPPPP/BRKRNBNQ w BDbd - 0 1",
"rknrbqnb/pppppppp/8/8/8/8/PPPPPPPP/RKNRBQNB w ADad - 0 1",
"bnqnrbkr/pppppppp/8/8/8/8/PPPPPPPP/BNQNRBKR w EHeh - 0 1",
"rbbnqkrn/pppppppp/8/8/8/8/PPPPPPPP/RBBNQKRN w AGag - 0 1",
"bqrkrnnb/pppppppp/8/8/8/8/PPPPPPPP/BQRKRNNB w CEce - 0 1",
"rkrnbbqn/pppppppp/8/8/8/8/PPPPPPPP/RKRNBBQN w ACac - 0 1",
"rbnkqnbr/pppppppp/8/8/8/8/PPPPPPPP/RBNKQNBR w AHah - 0 1",
"nbnrqkbr/pppppppp/8/8/8/8/PPPPPPPP/NBNRQKBR w DHdh - 0 1",
"nrkbnqbr/pppppppp/8/8/8/8/PPPPPPPP/NRKBNQBR w BHbh - 0 1",
"rknbbqrn/pppppppp/8/8/8/8/PPPPPPPP/RKNBBQRN w AGag - 0 1",
"rnkqnrbb/pppppppp/8/8/8/8/PPPPPPPP/RNKQNRBB w AFaf - 0 1",
"rnkbbqnr/pppppppp/8/8/8/8/PPPPPPPP/RNKBBQNR w AHah - 0 1",
"nrnbbqkr/pppppppp/8/8/8/8/PPPPPPPP/NRNBBQKR w BHbh - 0 1",
"rnbbnkqr/pppppppp/8/8/8/8/PPPPPPPP/RNBBNKQR w AHah - 0 1",
"bnqbrnkr/pppppppp/8/8/8/8/PPPPPPPP/BNQBRNKR w EHeh - 0 1",
"rnkbqrbn/pppppppp/8/8/8/8/PPPPPPPP/RNKBQRBN w AFaf - 0 1",
"rkbqnnrb/pppppppp/8/8/8/8/PPPPPPPP/RKBQNNRB w AGag - 0 1",
"nrbkqbnr/pppppppp/8/8/8/8/PPPPPPPP/NRBKQBNR w BHbh - 0 1",
"rkqnrbbn/pppppppp/8/8/8/8/PPPPPPPP/RKQNRBBN w AEae - 0 1",
"nnbrqbkr/pppppppp/8/8/8/8/PPPPPPPP/NNBRQBKR w DHdh - 0 1",
"rnbqknrb/pppppppp/8/8/8/8/PPPPPPPP/RNBQKNRB w AGag - 0 1",
"bnqnrkrb/pppppppp/8/8/8/8/PPPPPPPP/BNQNRKRB w EGeg - 0 1",
"bbrkqnnr/pppppppp/8/8/8/8/PPPPPPPP/BBRKQNNR w CHch - 0 1",
"brnnkqrb/pppppppp/8/8/8/8/PPPPPPPP/BRNNKQRB w BGbg - 0 1",
"rkrnbnqb/pppppppp/8/8/8/8/PPPPPPPP/RKRNBNQB w ACac - 0 1",
"bqrknnrb/pppppppp/8/8/8/8/PPPPPPPP/BQRKNNRB w CGcg - 0 1",
"rbbknnrq/pppppppp/8/8/8/8/PPPPPPPP/RBBKNNRQ w AGag - 0 1",
"bbrkrnqn/pppppppp/8/8/8/8/PPPPPPPP/BBRKRNQN w CEce - 0 1",
"rqnkbbnr/pppppppp/8/8/8/8/PPPPPPPP/RQNKBBNR w AHah - 0 1",
"rbbkqrnn/pppppppp/8/8/8/8/PPPPPPPP/RBBKQRNN w AFaf - 0 1",
"rqknnbbr/pppppppp/8/8/8/8/PPPPPPPP/RQKNNBBR w AHah - 0 1",
"rbqnkrbn/pppppppp/8/8/8/8/PPPPPPPP/RBQNKRBN w AFaf - 0 1",
"qbrnbnkr/pppppppp/8/8/8/8/PPPPPPPP/QBRNBNKR w CHch - 0 1",
"nqrbknbr/pppppppp/8/8/8/8/PPPPPPPP/NQRBKNBR w CHch - 0 1",
"rkbnqbrn/pppppppp/8/8/8/8/PPPPPPPP/RKBNQBRN w AGag - 0 1",
"bnqbrkrn/pppppppp/8/8/8/8/PPPPPPPP/BNQBRKRN w EGeg - 0 1",
"brnnkrqb/pppppppp/8/8/8/8/PPPPPPPP/BRNNKRQB w BFbf - 0 1",
"rnnqkrbb/pppppppp/8/8/8/8/PPPPPPPP/RNNQKRBB w AFaf - 0 1",
"nbnrbkrq/pppppppp/8/8/8/8/PPPPPPPP/NBNRBKRQ w DGdg - 0 1",
"rkrqnnbb/pppppppp/8/8/8/8/PPPPPPPP/RKRQNNBB w ACac - 0 1",
"brkbrnqn/pppppppp/8/8/8/8/PPPPPPPP/BRKBRNQN w BEbe - 0 1",
"nnrbbkrq/pppppppp/8/8/8/8/PPPPPPPP/NNRBBKRQ w CGcg - 0 1",
"rnqnbkrb/pppppppp/8/8/8/8/PPPPPPPP/RNQNBKRB w AGag - 0 1",
"nrbkrbnq/pppppppp/8/8/8/8/PPPPPPPP/NRBKRBNQ w BEbe - 0 1",
"qrbknrnb/pppppppp/8/8/8/8/PPPPPPPP/QRBKNRNB w BFbf - 0 1",
"nrknbbqr/pppppppp/8/8/8/8/PPPPPPPP/NRKNBBQR w BHbh - 0 1",
"qbbrnknr/pppppppp/8/8/8/8/PPPPPPPP/QBBRNKNR w DHdh - 0 1",
"bnrkrbnq/pppppppp/8/8/8/8/PPPPPPPP/BNRKRBNQ w CEce - 0 1",
"bnrbknqr/pppppppp/8/8/8/8/PPPPPPPP/BNRBKNQR w CHch - 0 1",
"rkqnnbbr/pppppppp/8/8/8/8/PPPPPPPP/RKQNNBBR w AHah - 0 1",
"bbrknqnr/pppppppp/8/8/8/8/PPPPPPPP/BBRKNQNR w CHch - 0 1",
"bnrqkbrn/pppppppp/8/8/8/8/PPPPPPPP/BNRQKBRN w CGcg - 0 1",
"nrbbkqnr/pppppppp/8/8/8/8/PPPPPPPP/NRBBKQNR w BHbh - 0 1",
"bqrknbnr/pppppppp/8/8/8/8/PPPPPPPP/BQRKNBNR w CHch - 0 1",
"nnrkbbqr/pppppppp/8/8/8/8/PPPPPPPP/NNRKBBQR w CHch - 0 1",
"rnbqkbrn/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBRN w AGag - 0 1",
"qrnnkrbb/pppppppp/8/8/8/8/PPPPPPPP/QRNNKRBB w BFbf - 0 1",
"rbbknqrn/pppppppp/8/8/8/8/PPPPPPPP/RBBKNQRN w AGag - 0 1",
"nbrqbkrn/pppppppp/8/8/8/8/PPPPPPPP/NBRQBKRN w CGcg - 0 1",
"brkbnqnr/pppppppp/8/8/8/8/PPPPPPPP/BRKBNQNR w BHbh - 0 1",
"bbnrnkrq/pppppppp/8/8/8/8/PPPPPPPP/BBNRNKRQ w DGdg - 0 1",
"rknrbbqn/pppppppp/8/8/8/8/PPPPPPPP/RKNRBBQN w ADad - 0 1",
"nrkrnbbq/pppppppp/8/8/8/8/PPPPPPPP/NRKRNBBQ w BDbd - 0 1",
"bnqrkrnb/pppppppp/8/8/8/8/PPPPPPPP/BNQRKRNB w DFdf - 0 1",
"rbbknqnr/pppppppp/8/8/8/8/PPPPPPPP/RBBKNQNR w AHah - 0 1",
"rqnkbbrn/pppppppp/8/8/8/8/PPPPPPPP/RQNKBBRN w AGag - 0 1",
"qnrbbnkr/pppppppp/8/8/8/8/PPPPPPPP/QNRBBNKR w CHch - 0 1",
"rnbkrbqn/pppppppp/8/8/8/8/PPPPPPPP/RNBKRBQN w AEae - 0 1",
"qrknnbbr/pppppppp/8/8/8/8/PPPPPPPP/QRKNNBBR w BHbh - 0 1",
"bbqrnkrn/pppppppp/8/8/8/8/PPPPPPPP/BBQRNKRN w DGdg - 0 1",
"nbrkqnbr/pppppppp/8/8/8/8/PPPPPPPP/NBRKQNBR w CHch - 0 1",
"bnrbkqnr/pppppppp/8/8/8/8/PPPPPPPP/BNRBKQNR w CHch - 0 1",
"nrkbqrbn/pppppppp/8/8/8/8/PPPPPPPP/NRKBQRBN w BFbf - 0 1",
"rkrbqnbn/pppppppp/8/8/8/8/PPPPPPPP/RKRBQNBN w ACac - 0 1",
"nbrkrqbn/pppppppp/8/8/8/8/PPPPPPPP/NBRKRQBN w CEce - 0 1",
"qrnbkrbn/pppppppp/8/8/8/8/PPPPPPPP/QRNBKRBN w BFbf - 0 1",
"rbnqbkrn/pppppppp/8/8/8/8/PPPPPPPP/RBNQBKRN w AGag - 0 1",
"nbqrnkbr/pppppppp/8/8/8/8/PPPPPPPP/NBQRNKBR w DHdh - 0 1",
"qrnbbkrn/pppppppp/8/8/8/8/PPPPPPPP/QRNBBKRN w BGbg - 0 1",
"rkbrnbqn/pppppppp/8/8/8/8/PPPPPPPP/RKBRNBQN w ADad - 0 1",
"brkbqnrn/pppppppp/8/8/8/8/PPPPPPPP/BRKBQNRN w BGbg - 0 1",
"rqnknrbb/pppppppp/8/8/8/8/PPPPPPPP/RQNKNRBB w AFaf - 0 1",
"nrknbbrq/pppppppp/8/8/8/8/PPPPPPPP/NRKNBBRQ w BGbg - 0 1",
"nnrkrbbq/pppppppp/8/8/8/8/PPPPPPPP/NNRKRBBQ w CEce - 0 1",
"qrnkbbnr/pppppppp/8/8/8/8/PPPPPPPP/QRNKBBNR w BHbh - 0 1",
"bbnrnkqr/pppppppp/8/8/8/8/PPPPPPPP/BBNRNKQR w DHdh - 0 1",
"rkbnqnrb/pppppppp/8/8/8/8/PPPPPPPP/RKBNQNRB w AGag - 0 1",
"qrbbnkrn/pppppppp/8/8/8/8/PPPPPPPP/QRBBNKRN w BGbg - 0 1",
"qnnrbbkr/pppppppp/8/8/8/8/PPPPPPPP/QNNRBBKR w DHdh - 0 1",
"rqkbrnbn/pppppppp/8/8/8/8/PPPPPPPP/RQKBRNBN w AEae - 0 1",
"rknnbbqr/pppppppp/8/8/8/8/PPPPPPPP/RKNNBBQR w AHah - 0 1",
"bnrkrqnb/pppppppp/8/8/8/8/PPPPPPPP/BNRKRQNB w CEce - 0 1",
"bnnrkbrq/pppppppp/8/8/8/8/PPPPPPPP/BNNRKBRQ w DGdg - 0 1",
"rknbnrbq/pppppppp/8/8/8/8/PPPPPPPP/RKNBNRBQ w AFaf - 0 1",
"rqkbnrbn/pppppppp/8/8/8/8/PPPPPPPP/RQKBNRBN w AFaf - 0 1",
"nrkbbqrn/pppppppp/8/8/8/8/PPPPPPPP/NRKBBQRN w BGbg - 0 1",
"rqbbnknr/pppppppp/8/8/8/8/PPPPPPPP/RQBBNKNR w AHah - 0 1",
"rkqbbrnn/pppppppp/8/8/8/8/PPPPPPPP/RKQBBRNN w AFaf - 0 1",
"bbnrknrq/pppppppp/8/8/8/8/PPPPPPPP/BBNRKNRQ w DGdg - 0 1",
"rknqrbbn/pppppppp/8/8/8/8/PPPPPPPP/RKNQRBBN w AEae - 0 1",
"bnrknbrq/pppppppp/8/8/8/8/PPPPPPPP/BNRKNBRQ w CGcg - 0 1",
"qbbnrnkr/pppppppp/8/8/8/8/PPPPPPPP/QBBNRNKR w EHeh - 0 1",
"qrnknrbb/pppppppp/8/8/8/8/PPPPPPPP/QRNKNRBB w BFbf - 0 1",
"rkbrnqnb/pppppppp/8/8/8/8/PPPPPPPP/RKBRNQNB w ADad - 0 1",
"bbrnqnkr/pppppppp/8/8/8/8/PPPPPPPP/BBRNQNKR w CHch - 0 1",
"rbbknrqn/pppppppp/8/8/8/8/PPPPPPPP/RBBKNRQN w AFaf - 0 1",
"nrqknbbr/pppppppp/8/8/8/8/PPPPPPPP/NRQKNBBR w BHbh - 0 1",
"bqrnnkrb/pppppppp/8/8/8/8/PPPPPPPP/BQRNNKRB w CGcg - 0 1",
"nrqknrbb/pppppppp/8/8/8/8/PPPPPPPP/NRQKNRBB w BFbf - 0 1",
"qrkrbnnb/pppppppp/8/8/8/8/PPPPPPPP/QRKRBNNB w BDbd - 0 1",
"nrbbnqkr/pppppppp/8/8/8/8/PPPPPPPP/NRBBNQKR w BHbh - 0 1",
"rbknrqbn/pppppppp/8/8/8/8/PPPPPPPP/RBKNRQBN w AEae - 0 1",
"bbrnknrq/pppppppp/8/8/8/8/PPPPPPPP/BBRNKNRQ w CGcg - 0 1",
"nbrknqbr/pppppppp/8/8/8/8/PPPPPPPP/NBRKNQBR w CHch - 0 1",
"rnkrnbbq/pppppppp/8/8/8/8/PPPPPPPP/RNKRNBBQ w ADad - 0 1",
"brqknnrb/pppppppp/8/8/8/8/PPPPPPPP/BRQKNNRB w BGbg - 0 1",
"rnkrnqbb/pppppppp/8/8/8/8/PPPPPPPP/RNKRNQBB w ADad - 0 1",
"nrbkrqnb/pppppppp/8/8/8/8/PPPPPPPP/NRBKRQNB w BEbe - 0 1",
"rkbnnqrb/pppppppp/8/8/8/8/PPPPPPPP/RKBNNQRB w AGag - 0 1",
"nnrbqkbr/pppppppp/8/8/8/8/PPPPPPPP/NNRBQKBR w CHch - 0 1",
"brnbnkqr/pppppppp/8/8/8/8/PPPPPPPP/BRNBNKQR w BHbh - 0 1",
"qrbnkbnr/pppppppp/8/8/8/8/PPPPPPPP/QRBNKBNR w BHbh - 0 1",
"rnqbbknr/pppppppp/8/8/8/8/PPPPPPPP/RNQBBKNR w AHah - 0 1",
"rbnnkrbq/pppppppp/8/8/8/8/PPPPPPPP/RBNNKRBQ w AFaf - 0 1",
"rqbnkrnb/pppppppp/8/8/8/8/PPPPPPPP/RQBNKRNB w AFaf - 0 1",
"rnknbrqb/pppppppp/8/8/8/8/PPPPPPPP/RNKNBRQB w AFaf - 0 1",
"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah - 0 1",
"qbnnbrkr/pppppppp/8/8/8/8/PPPPPPPP/QBNNBRKR w FHfh - 0 1",
"rknbqrbn/pppppppp/8/8/8/8/PPPPPPPP/RKNBQRBN w AFaf - 0 1",
"qnbrkbrn/pppppppp/8/8/8/8/PPPPPPPP/QNBRKBRN w DGdg - 0 1",
"rnkbbnqr/pppppppp/8/8/8/8/PPPPPPPP/RNKBBNQR w AHah - 0 1",
"rqkrbbnn/pppppppp/8/8/8/8/PPPPPPPP/RQKRBBNN w ADad - 0 1",
"qbbnrknr/pppppppp/8/8/8/8/PPPPPPPP/QBBNRKNR w EHeh - 0 1",
"rbnknqbr/pppppppp/8/8/8/8/PPPPPPPP/RBNKNQBR w AHah - 0 1",
"rkbbqnnr/pppppppp/8/8/8/8/PPPPPPPP/RKBBQNNR w AHah - 0 1",
"nnqrkbbr/pppppppp/8/8/8/8/PPPPPPPP/NNQRKBBR w DHdh - 0 1",
"bqrknrnb/pppppppp/8/8/8/8/PPPPPPPP/BQRKNRNB w CFcf - 0 1",
"rqbbknrn/pppppppp/8/8/8/8/PPPPPPPP/RQBBKNRN w AGag - 0 1",
"rkbbnqrn/pppppppp/8/8/8/8/PPPPPPPP/RKBBNQRN w AGag - 0 1",
"brkbnnqr/pppppppp/8/8/8/8/PPPPPPPP/BRKBNNQR w BHbh - 0 1",
"nrqnkbbr/pppppppp/8/8/8/8/PPPPPPPP/NRQNKBBR w BHbh - 0 1",
"bnnrkqrb/pppppppp/8/8/8/8/PPPPPPPP/BNNRKQRB w DGdg - 0 1",
"nqbrkbnr/pppppppp/8/8/8/8/PPPPPPPP/NQBRKBNR w DHdh - 0 1",
"bbrkrnnq/pppppppp/8/8/8/8/PPPPPPPP/BBRKRNNQ w CEce - 0 1",
"nqnrkbbr/pppppppp/8/8/8/8/PPPPPPPP/NQNRKBBR w DHdh - 0 1",
"bnqrnbkr/pppppppp/8/8/8/8/PPPPPPPP/BNQRNBKR w DHdh - 0 1",
"rknqbrnb/pppppppp/8/8/8/8/PPPPPPPP/RKNQBRNB w AFaf - 0 1",
"nnbrkbqr/pppppppp/8/8/8/8/PPPPPPPP/NNBRKBQR w DHdh - 0 1",
"rnbkrqnb/pppppppp/8/8/8/8/PPPPPPPP/RNBKRQNB w AEae - 0 1",
"brknrqnb/pppppppp/8/8/8/8/PPPPPPPP/BRKNRQNB w BEbe - 0 1",
"bbnnrqkr/pppppppp/8/8/8/8/PPPPPPPP/BBNNRQKR w EHeh - 0 1",
"rknbrnbq/pppppppp/8/8/8/8/PPPPPPPP/RKNBRNBQ w AEae - 0 1",
"rnqkrnbb/pppppppp/8/8/8/8/PPPPPPPP/RNQKRNBB w AEae - 0 1",
"rknnqbbr/pppppppp/8/8/8/8/PPPPPPPP/RKNNQBBR w AHah - 0 1",
"nqrbbkrn/pppppppp/8/8/8/8/PPPPPPPP/NQRBBKRN w CGcg - 0 1",
"brkrnbqn/pppppppp/8/8/8/8/PPPPPPPP/BRKRNBQN w BDbd - 0 1",
"bqrnkbnr/pppppppp/8/8/8/8/PPPPPPPP/BQRNKBNR w CHch - 0 1",
"nrkbrqbn/pppppppp/8/8/8/8/PPPPPPPP/NRKBRQBN w BEbe - 0 1",
"bbrnqknr/pppppppp/8/8/8/8/PPPPPPPP/BBRNQKNR w CHch - 0 1",
"rnbknbrq/pppppppp/8/8/8/8/PPPPPPPP/RNBKNBRQ w AGag - 0 1",
"bnqbrknr/pppppppp/8/8/8/8/PPPPPPPP/BNQBRKNR w EHeh - 0 1",
"rnkqbrnb/pppppppp/8/8/8/8/PPPPPPPP/RNKQBRNB w AFaf - 0 1",
"rqbknrnb/pppppppp/8/8/8/8/PPPPPPPP/RQBKNRNB w AFaf - 0 1",
"rbnqknbr/pppppppp/8/8/8/8/PPPPPPPP/RBNQKNBR w AHah - 0 1",
"rbbnnqkr/pppppppp/8/8/8/8/PPPPPPPP/RBBNNQKR w AHah - 0 1",
"rnbnqkrb/pppppppp/8/8/8/8/PPPPPPPP/RNBNQKRB w AGag - 0 1",
"bnqrkbrn/pppppppp/8/8/8/8/PPPPPPPP/BNQRKBRN w DGdg - 0 1",
"brnkrnqb/pppppppp/8/8/8/8/PPPPPPPP/BRNKRNQB w BEbe - 0 1",
"rkrbbnnq/pppppppp/8/8/8/8/PPPPPPPP/RKRBBNNQ w ACac - 0 1",
"brnnkbrq/pppppppp/8/8/8/8/PPPPPPPP/BRNNKBRQ w BGbg - 0 1",
"nrqbkrbn/pppppppp/8/8/8/8/PPPPPPPP/NRQBKRBN w BFbf - 0 1",
"nnrqbkrb/pppppppp/8/8/8/8/PPPPPPPP/NNRQBKRB w CGcg - 0 1",
"rknnrbbq/pppppppp/8/8/8/8/PPPPPPPP/RKNNRBBQ w AEae - 0 1",
"nrkbbnrq/pppppppp/8/8/8/8/PPPPPPPP/NRKBBNRQ w BGbg - 0 1",
"bbrknrnq/pppppppp/8/8/8/8/PPPPPPPP/BBRKNRNQ w CFcf - 0 1",
"rnqkbnrb/pppppppp/8/8/8/8/PPPPPPPP/RNQKBNRB w AGag - 0 1",
"qrknbbnr/pppppppp/8/8/8/8/PPPPPPPP/QRKNBBNR w BHbh - 0 1",
"brqknrnb/pppppppp/8/8/8/8/PPPPPPPP/BRQKNRNB w BFbf - 0 1",
"bnrbnkrq/pppppppp/8/8/8/8/PPPPPPPP/BNRBNKRQ w CGcg - 0 1",
"bnrkqnrb/pppppppp/8/8/8/8/PPPPPPPP/BNRKQNRB w CGcg - 0 1",
"bqrbnkrn/pppppppp/8/8/8/8/PPPPPPPP/BQRBNKRN w CGcg - 0 1",
"brnbnkrq/pppppppp/8/8/8/8/PPPPPPPP/BRNBNKRQ w BGbg - 0 1",
"nbnqrkbr/pppppppp/8/8/8/8/PPPPPPPP/NBNQRKBR w EHeh - 0 1",
"rnnbkqbr/pppppppp/8/8/8/8/PPPPPPPP/RNNBKQBR w AHah - 0 1",
"rbkqrnbn/pppppppp/8/8/8/8/PPPPPPPP/RBKQRNBN w AEae - 0 1",
"qrknrnbb/pppppppp/8/8/8/8/PPPPPPPP/QRKNRNBB w BEbe - 0 1",
"rqnbbknr/pppppppp/8/8/8/8/PPPPPPPP/RQNBBKNR w AHah - 0 1",
"rnbbknrq/pppppppp/8/8/8/8/PPPPPPPP/RNBBKNRQ w AGag - 0 1",
"qbrnknbr/pppppppp/8/8/8/8/PPPPPPPP/QBRNKNBR w CHch - 0 1",
"nqrnkrbb/pppppppp/8/8/8/8/PPPPPPPP/NQRNKRBB w CFcf - 0 1",
"qnrkbnrb/pppppppp/8/8/8/8/PPPPPPPP/QNRKBNRB w CGcg - 0 1",
"brqkrbnn/pppppppp/8/8/8/8/PPPPPPPP/BRQKRBNN w BEbe - 0 1",
"qbrnbknr/pppppppp/8/8/8/8/PPPPPPPP/QBRNBKNR w CHch - 0 1",
"nrnkbbqr/pppppppp/8/8/8/8/PPPPPPPP/NRNKBBQR w BHbh - 0 1",
"rqkrnnbb/pppppppp/8/8/8/8/PPPPPPPP/RQKRNNBB w ADad - 0 1",
"rqnnkbbr/pppppppp/8/8/8/8/PPPPPPPP/RQNNKBBR w AHah - 0 1",
"rkbnnrqb/pppppppp/8/8/8/8/PPPPPPPP/RKBNNRQB w AFaf - 0 1",
"nqbnrbkr/pppppppp/8/8/8/8/PPPPPPPP/NQBNRBKR w EHeh - 0 1",
"rkbnrbqn/pppppppp/8/8/8/8/PPPPPPPP/RKBNRBQN w AEae - 0 1",
"bnrbqnkr/pppppppp/8/8/8/8/PPPPPPPP/BNRBQNKR w CHch - 0 1",
"brnqkbrn/pppppppp/8/8/8/8/PPPPPPPP/BRNQKBRN w BGbg - 0 1",
"rqbnnkrb/pppppppp/8/8/8/8/PPPPPPPP/RQBNNKRB w AGag - 0 1",
"nbbrnkrq/pppppppp/8/8/8/8/PPPPPPPP/NBBRNKRQ w DGdg - 0 1",
"bnrbkrnq/pppppppp/8/8/8/8/PPPPPPPP/BNRBKRNQ w CFcf - 0 1",
"nrbnkrqb/pppppppp/8/8/8/8/PPPPPPPP/NRBNKRQB w BFbf - 0 1",
"brqnknrb/pppppppp/8/8/8/8/PPPPPPPP/BRQNKNRB w BGbg - 0 1",
"rqknrbbn/pppppppp/8/8/8/8/PPPPPPPP/RQKNRBBN w AEae - 0 1",
"bqrbknrn/pppppppp/8/8/8/8/PPPPPPPP/BQRBKNRN w CGcg - 0 1",
"brkbnnrq/pppppppp/8/8/8/8/PPPPPPPP/BRKBNNRQ w BGbg - 0 1",
"qrbknbnr/pppppppp/8/8/8/8/PPPPPPPP/QRBKNBNR w BHbh - 0 1",
"rnnqbbkr/pppppppp/8/8/8/8/PPPPPPPP/RNNQBBKR w AHah - 0 1",
"rnbbqknr/pppppppp/8/8/8/8/PPPPPPPP/RNBBQKNR w AHah - 0 1",
"bbnnqrkr/pppppppp/8/8/8/8/PPPPPPPP/BBNNQRKR w FHfh - 0 1",
"qnbbrknr/pppppppp/8/8/8/8/PPPPPPPP/QNBBRKNR w EHeh - 0 1",
"nrbkrbqn/pppppppp/8/8/8/8/PPPPPPPP/NRBKRBQN w BEbe - 0 1",
"nqbbrkrn/pppppppp/8/8/8/8/PPPPPPPP/NQBBRKRN w EGeg - 0 1",
"rnnbkrbq/pppppppp/8/8/8/8/PPPPPPPP/RNNBKRBQ w AFaf - 0 1",
"bqrkrbnn/pppppppp/8/8/8/8/PPPPPPPP/BQRKRBNN w CEce - 0 1",
"rnbbkrnq/pppppppp/8/8/8/8/PPPPPPPP/RNBBKRNQ w AFaf - 0 1",
"nrbnqkrb/pppppppp/8/8/8/8/PPPPPPPP/NRBNQKRB w BGbg - 0 1",
"rbqkbnnr/pppppppp/8/8/8/8/PPPPPPPP/RBQKBNNR w AHah - 0 1",
"rbnqnkbr/pppppppp/8/8/8/8/PPPPPPPP/RBNQNKBR w AHah - 0 1",
"nrnqkrbb/pppppppp/8/8/8/8/PPPPPPPP/NRNQKRBB w BFbf - 0 1",
"rkbbnrnq/pppppppp/8/8/8/8/PPPPPPPP/RKBBNRNQ w AFaf - 0 1",
"qnnbrkbr/pppppppp/8/8/8/8/PPPPPPPP/QNNBRKBR w EHeh - 0 1",
"brqnnbkr/pppppppp/8/8/8/8/PPPPPPPP/BRQNNBKR w BHbh - 0 1",
"qbrknrbn/pppppppp/8/8/8/8/PPPPPPPP/QBRKNRBN w CFcf - 0 1",
"nrbnkbrq/pppppppp/8/8/8/8/PPPPPPPP/NRBNKBRQ w BGbg - 0 1",
"bqrbkrnn/pppppppp/8/8/8/8/PPPPPPPP/BQRBKRNN w CFcf - 0 1",
"bnqrnkrb/pppppppp/8/8/8/8/PPPPPPPP/BNQRNKRB w DGdg - 0 1",
"bnqrknrb/pppppppp/8/8/8/8/PPPPPPPP/BNQRKNRB w DGdg - 0 1",
"qnrkbrnb/pppppppp/8/8/8/8/PPPPPPPP/QNRKBRNB w CFcf - 0 1",
"nrkbbnqr/pppppppp/8/8/8/8/PPPPPPPP/NRKBBNQR w BHbh - 0 1",
"rnbkrnqb/pppppppp/8/8/8/8/PPPPPPPP/RNBKRNQB w AEae - 0 1",
"rbqknnbr/pppppppp/8/8/8/8/PPPPPPPP/RBQKNNBR w AHah - 0 1",
"rkbnrbnq/pppppppp/8/8/8/8/PPPPPPPP/RKBNRBNQ w AEae - 0 1",
"nrnkbrqb/pppppppp/8/8/8/8/PPPPPPPP/NRNKBRQB w BFbf - 0 1",
"brnbkqnr/pppppppp/8/8/8/8/PPPPPPPP/BRNBKQNR w BHbh - 0 1",
"rnnbbqkr/pppppppp/8/8/8/8/PPPPPPPP/RNNBBQKR w AHah - 0 1",
"rnnkbbrq/pppppppp/8/8/8/8/PPPPPPPP/RNNKBBRQ w AGag - 0 1",
"rnbkrbnq/pppppppp/8/8/8/8/PPPPPPPP/RNBKRBNQ w AEae - 0 1",
"brnbqkrn/pppppppp/8/8/8/8/PPPPPPPP/BRNBQKRN w BGbg - 0 1",
"nbbrnkqr/pppppppp/8/8/8/8/PPPPPPPP/NBBRNKQR w DHdh - 0 1",
"bbnqnrkr/pppppppp/8/8/8/8/PPPPPPPP/BBNQNRKR w FHfh - 0 1",
"nrkbbrnq/pppppppp/8/8/8/8/PPPPPPPP/NRKBBRNQ w BFbf - 0 1",
"rnbkqbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBKQBNR w AHah - 0 1",
"brkbnqrn/pppppppp/8/8/8/8/PPPPPPPP/BRKBNQRN w BGbg - 0 1",
"nrqbbnkr/pppppppp/8/8/8/8/PPPPPPPP/NRQBBNKR w BHbh - 0 1",
"qrkrbbnn/pppppppp/8/8/8/8/PPPPPPPP/QRKRBBNN w BDbd - 0 1",
"qrnbbnkr/pppppppp/8/8/8/8/PPPPPPPP/QRNBBNKR w BHbh - 0 1",
"nrbknqrb/pppppppp/8/8/8/8/PPPPPPPP/NRBKNQRB w BGbg - 0 1",
"bbrqnnkr/pppppppp/8/8/8/8/PPPPPPPP/BBRQNNKR w CHch - 0 1",
"rbkrbnqn/pppppppp/8/8/8/8/PPPPPPPP/RBKRBNQN w ADad - 0 1",
"brqnkbnr/pppppppp/8/8/8/8/PPPPPPPP/BRQNKBNR w BHbh - 0 1",
"rqbbnkrn/pppppppp/8/8/8/8/PPPPPPPP/RQBBNKRN w AGag - 0 1",
"bnrknrqb/pppppppp/8/8/8/8/PPPPPPPP/BNRKNRQB w CFcf - 0 1",
"nbbqnrkr/pppppppp/8/8/8/8/PPPPPPPP/NBBQNRKR w FHfh - 0 1",
"nbrkbnqr/pppppppp/8/8/8/8/PPPPPPPP/NBRKBNQR w CHch - 0 1",
"qnrkbbrn/pppppppp/8/8/8/8/PPPPPPPP/QNRKBBRN w CGcg - 0 1",
"brnqknrb/pppppppp/8/8/8/8/PPPPPPPP/BRNQKNRB w BGbg - 0 1",
"nbrqkrbn/pppppppp/8/8/8/8/PPPPPPPP/NBRQKRBN w CFcf - 0 1",
"nbrkqrbn/pppppppp/8/8/8/8/PPPPPPPP/NBRKQRBN w CFcf - 0 1",
"bbrnkqnr/pppppppp/8/8/8/8/PPPPPPPP/BBRNKQNR w CHch - 0 1",
"qrbbnknr/pppppppp/8/8/8/8/PPPPPPPP/QRBBNKNR w BHbh - 0 1",
"rbbqnknr/pppppppp/8/8/8/8/PPPPPPPP/RBBQNKNR w AHah - 0 1",
"bqrbnnkr/pppppppp/8/8/8/8/PPPPPPPP/BQRBNNKR w CHch - 0 1",
"rbnqbnkr/pppppppp/8/8/8/8/PPPPPPPP/RBNQBNKR w AHah - 0 1",
"nrkrbnqb/pppppppp/8/8/8/8/PPPPPPPP/NRKRBNQB w BDbd - 0 1",
"rbkqbnnr/pppppppp/8/8/8/8/PPPPPPPP/RBKQBNNR w AHah - 0 1",
"nbrnqkbr/pppppppp/8/8/8/8/PPPPPPPP/NBRNQKBR w CHch - 0 1",
"nrqkrbbn/pppppppp/8/8/8/8/PPPPPPPP/NRQKRBBN w BEbe - 0 1",
"qnrbnkbr/pppppppp/8/8/8/8/PPPPPPPP/QNRBNKBR w CHch - 0 1",
"qnrknbbr/pppppppp/8/8/8/8/PPPPPPPP/QNRKNBBR w CHch - 0 1",
"nrqnbkrb/pppppppp/8/8/8/8/PPPPPPPP/NRQNBKRB w BGbg - 0 1",
"nbrkrnbq/pppppppp/8/8/8/8/PPPPPPPP/NBRKRNBQ w CEce - 0 1",
"nqrbbknr/pppppppp/8/8/8/8/PPPPPPPP/NQRBBKNR w CHch - 0 1",
"rbnnbqkr/pppppppp/8/8/8/8/PPPPPPPP/RBNNBQKR w AHah - 0 1",
"nbnrbkqr/pppppppp/8/8/8/8/PPPPPPPP/NBNRBKQR w DHdh - 0 1",
"rbqkbnrn/pppppppp/8/8/8/8/PPPPPPPP/RBQKBNRN w AGag - 0 1",
"nrnkrbbq/pppppppp/8/8/8/8/PPPPPPPP/NRNKRBBQ w BEbe - 0 1",
"rbnnbkqr/pppppppp/8/8/8/8/PPPPPPPP/RBNNBKQR w AHah - 0 1",
"qrkbbnnr/pppppppp/8/8/8/8/PPPPPPPP/QRKBBNNR w BHbh - 0 1",
"nrbkqnrb/pppppppp/8/8/8/8/PPPPPPPP/NRBKQNRB w BGbg - 0 1",
"bnnbrqkr/pppppppp/8/8/8/8/PPPPPPPP/BNNBRQKR w EHeh - 0 1",
"rbbnkqnr/pppppppp/8/8/8/8/PPPPPPPP/RBBNKQNR w AHah - 0 1",
"rknnrqbb/pppppppp/8/8/8/8/PPPPPPPP/RKNNRQBB w AEae - 0 1",
"rbbnqnkr/pppppppp/8/8/8/8/PPPPPPPP/RBBNQNKR w AHah - 0 1",
"bnrbknrq/pppppppp/8/8/8/8/PPPPPPPP/BNRBKNRQ w CGcg - 0 1",
"brnqkbnr/pppppppp/8/8/8/8/PPPPPPPP/BRNQKBNR w BHbh - 0 1",
"brkbnrnq/pppppppp/8/8/8/8/PPPPPPPP/BRKBNRNQ w BFbf - 0 1",
"rkrqbbnn/pppppppp/8/8/8/8/PPPPPPPP/RKRQBBNN w ACac - 0 1",
"nbbqrnkr/pppppppp/8/8/8/8/PPPPPPPP/NBBQRNKR w EHeh - 0 1",
"rkqnnrbb/pppppppp/8/8/8/8/PPPPPPPP/RKQNNRBB w AFaf - 0 1",
"brkqnrnb/pppppppp/8/8/8/8/PPPPPPPP/BRKQNRNB w BFbf - 0 1",
"bnrbkqrn/pppppppp/8/8/8/8/PPPPPPPP/BNRBKQRN w CGcg - 0 1",
"brkrqnnb/pppppppp/8/8/8/8/PPPPPPPP/BRKRQNNB w BDbd - 0 1",
"bbrqkrnn/pppppppp/8/8/8/8/PPPPPPPP/BBRQKRNN w CFcf - 0 1",
"rnbbqkrn/pppppppp/8/8/8/8/PPPPPPPP/RNBBQKRN w AGag - 0 1",
"nbqrbknr/pppppppp/8/8/8/8/PPPPPPPP/NBQRBKNR w DHdh - 0 1",
"nbrkbqrn/pppppppp/8/8/8/8/PPPPPPPP/NBRKBQRN w CGcg - 0 1",
"rbbknnqr/pppppppp/8/8/8/8/PPPPPPPP/RBBKNNQR w AHah - 0 1",
"qrbnnkrb/pppppppp/8/8/8/8/PPPPPPPP/QRBNNKRB w BGbg - 0 1",
"bbrnnkqr/pppppppp/8/8/8/8/PPPPPPPP/BBRNNKQR w CHch - 0 1",
"nrbbknqr/pppppppp/8/8/8/8/PPPPPPPP/NRBBKNQR w BHbh - 0 1",
"brnbqknr/pppppppp/8/8/8/8/PPPPPPPP/BRNBQKNR w BHbh - 0 1",
"brknqnrb/pppppppp/8/8/8/8/PPPPPPPP/BRKNQNRB w BGbg - 0 1",
"nrbnqbkr/pppppppp/8/8/8/8/PPPPPPPP/NRBNQBKR w BHbh - 0 1",
"rkqrnnbb/pppppppp/8/8/8/8/PPPPPPPP/RKQRNNBB w ADad - 0 1",
"brkbqrnn/pppppppp/8/8/8/8/PPPPPPPP/BRKBQRNN w BFbf - 0 1",
"qnnrkrbb/pppppppp/8/8/8/8/PPPPPPPP/QNNRKRBB w DFdf - 0 1",
"brnbknqr/pppppppp/8/8/8/8/PPPPPPPP/BRNBKNQR w BHbh - 0 1",
"nqrknrbb/pppppppp/8/8/8/8/PPPPPPPP/NQRKNRBB w CFcf - 0 1",
"bqnrnbkr/pppppppp/8/8/8/8/PPPPPPPP/BQNRNBKR w DHdh - 0 1",
"bqnrkrnb/pppppppp/8/8/8/8/PPPPPPPP/BQNRKRNB w DFdf - 0 1",
"rbknqrbn/pppppppp/8/8/8/8/PPPPPPPP/RBKNQRBN w AFaf - 0 1",
"bbnrnqkr/pppppppp/8/8/8/8/PPPPPPPP/BBNRNQKR w DHdh - 0 1",
"nbbrqknr/pppppppp/8/8/8/8/PPPPPPPP/NBBRQKNR w DHdh - 0 1",
"bbrnnqkr/pppppppp/8/8/8/8/PPPPPPPP/BBRNNQKR w CHch - 0 1",
"bbrnqkrn/pppppppp/8/8/8/8/PPPPPPPP/BBRNQKRN w CGcg - 0 1",
"rqbknbrn/pppppppp/8/8/8/8/PPPPPPPP/RQBKNBRN w AGag - 0 1",
"brknnqrb/pppppppp/8/8/8/8/PPPPPPPP/BRKNNQRB w BGbg - 0 1",
"rknrqnbb/pppppppp/8/8/8/8/PPPPPPPP/RKNRQNBB w ADad - 0 1",
"nbqrknbr/pppppppp/8/8/8/8/PPPPPPPP/NBQRKNBR w DHdh - 0 1",
"nrkqnbbr/pppppppp/8/8/8/8/PPPPPPPP/NRKQNBBR w BHbh - 0 1",
"rkrnnbbq/pppppppp/8/8/8/8/PPPPPPPP/RKRNNBBQ w ACac - 0 1",
"nrbqkrnb/pppppppp/8/8/8/8/PPPPPPPP/NRBQKRNB w BFbf - 0 1",
"qbnrnkbr/pppppppp/8/8/8/8/PPPPPPPP/QBNRNKBR w DHdh - 0 1",
"nrqkbrnb/pppppppp/8/8/8/8/PPPPPPPP/NRQKBRNB w BFbf - 0 1",
"brnqkrnb/pppppppp/8/8/8/8/PPPPPPPP/BRNQKRNB w BFbf - 0 1",
"nqbrkbrn/pppppppp/8/8/8/8/PPPPPPPP/NQBRKBRN w DGdg - 0 1",
"nrkrbbqn/pppppppp/8/8/8/8/PPPPPPPP/NRKRBBQN w BDbd - 0 1",
"rknbqnbr/pppppppp/8/8/8/8/PPPPPPPP/RKNBQNBR w AHah - 0 1",
"bbnqrknr/pppppppp/8/8/8/8/PPPPPPPP/BBNQRKNR w EHeh - 0 1",
"rnbnkbrq/pppppppp/8/8/8/8/PPPPPPPP/RNBNKBRQ w AGag - 0 1",
"nrkqrbbn/pppppppp/8/8/8/8/PPPPPPPP/NRKQRBBN w BEbe - 0 1",
"nbbrknqr/pppppppp/8/8/8/8/PPPPPPPP/NBBRKNQR w DHdh - 0 1",
"bnrbkrqn/pppppppp/8/8/8/8/PPPPPPPP/BNRBKRQN w CFcf - 0 1",
"bbrkrqnn/pppppppp/8/8/8/8/PPPPPPPP/BBRKRQNN w CEce - 0 1",
"rbnkbnrq/pppppppp/8/8/8/8/PPPPPPPP/RBNKBNRQ w AGag - 0 1",
"rqnnbbkr/pppppppp/8/8/8/8/PPPPPPPP/RQNNBBKR w AHah - 0 1",
"bnrbnqkr/pppppppp/8/8/8/8/PPPPPPPP/BNRBNQKR w CHch - 0 1",
"rqnknbbr/pppppppp/8/8/8/8/PPPPPPPP/RQNKNBBR w AHah - 0 1",
"bbqnrnkr/pppppppp/8/8/8/8/PPPPPPPP/BBQNRNKR w EHeh - 0 1",
"rkrnbbnq/pppppppp/8/8/8/8/PPPPPPPP/RKRNBBNQ w ACac - 0 1",
"brnbkrnq/pppppppp/8/8/8/8/PPPPPPPP/BRNBKRNQ w BFbf - 0 1",
"rnnqbkrb/pppppppp/8/8/8/8/PPPPPPPP/RNNQBKRB w AGag - 0 1",
"brqkrnnb/pppppppp/8/8/8/8/PPPPPPPP/BRQKRNNB w BEbe - 0 1",
"nnrbbqkr/pppppppp/8/8/8/8/PPPPPPPP/NNRBBQKR w CHch - 0 1",
"rqbbkrnn/pppppppp/8/8/8/8/PPPPPPPP/RQBBKRNN w AFaf - 0 1",
"nbnqbrkr/pppppppp/8/8/8/8/PPPPPPPP/NBNQBRKR w FHfh - 0 1",
"nnrkrqbb/pppppppp/8/8/8/8/PPPPPPPP/NNRKRQBB w CEce - 0 1",
"nnbbqrkr/pppppppp/8/8/8/8/PPPPPPPP/NNBBQRKR w FHfh - 0 1",
"rnqkbbnr/pppppppp/8/8/8/8/PPPPPPPP/RNQKBBNR w AHah - 0 1",
"nbrnbkqr/pppppppp/8/8/8/8/PPPPPPPP/NBRNBKQR w CHch - 0 1",
"qbbrkrnn/pppppppp/8/8/8/8/PPPPPPPP/QBBRKRNN w DFdf - 0 1",
"rknbbnrq/pppppppp/8/8/8/8/PPPPPPPP/RKNBBNRQ w AGag - 0 1",
"rbnkbnqr/pppppppp/8/8/8/8/PPPPPPPP/RBNKBNQR w AHah - 0 1",
"bqnbrknr/pppppppp/8/8/8/8/PPPPPPPP/BQNBRKNR w EHeh - 0 1",
"rkqbbnrn/pppppppp/8/8/8/8/PPPPPPPP/RKQBBNRN w AGag - 0 1",
"rknqnrbb/pppppppp/8/8/8/8/PPPPPPPP/RKNQNRBB w AFaf - 0 1",
"rnnkbrqb/pppppppp/8/8/8/8/PPPPPPPP/RNNKBRQB w AFaf - 0 1",
"bbrknnrq/pppppppp/8/8/8/8/PPPPPPPP/BBRKNNRQ w CGcg - 0 1",
"rbkqnrbn/pppppppp/8/8/8/8/PPPPPPPP/RBKQNRBN w AFaf - 0 1",
"nrqkbnrb/pppppppp/8/8/8/8/PPPPPPPP/NRQKBNRB w BGbg - 0 1",
"rkbbnrqn/pppppppp/8/8/8/8/PPPPPPPP/RKBBNRQN w AFaf - 0 1",
"bbnrqknr/pppppppp/8/8/8/8/PPPPPPPP/BBNRQKNR w DHdh - 0 1",
"qrnnbkrb/pppppppp/8/8/8/8/PPPPPPPP/QRNNBKRB w BGbg - 0 1",
"qnbrnkrb/pppppppp/8/8/8/8/PPPPPPPP/QNBRNKRB w DGdg - 0 1",
"nrkbbqnr/pppppppp/8/8/8/8/PPPPPPPP/NRKBBQNR w BHbh - 0 1",
"rbnqkrbn/pppppppp/8/8/8/8/PPPPPPPP/RBNQKRBN w AFaf - 0 1",
"rbbnnkrq/pppppppp/8/8/8/8/PPPPPPPP/RBBNNKRQ w AGag - 0 1",
"qrnnkbbr/pppppppp/8/8/8/8/PPPPPPPP/QRNNKBBR w BHbh - 0 1",
"nnbqrkrb/pppppppp/8/8/8/8/PPPPPPPP/NNBQRKRB w EGeg - 0 1",
"brknqbnr/pppppppp/8/8/8/8/PPPPPPPP/BRKNQBNR w BHbh - 0 1",
"qrkrnbbn/pppppppp/8/8/8/8/PPPPPPPP/QRKRNBBN w BDbd - 0 1",
"rbknbqnr/pppppppp/8/8/8/8/PPPPPPPP/RBKNBQNR w AHah - 0 1",
"rnqnkrbb/pppppppp/8/8/8/8/PPPPPPPP/RNQNKRBB w AFaf - 0 1",
"rnkbbrnq/pppppppp/8/8/8/8/PPPPPPPP/RNKBBRNQ w AFaf - 0 1",
"nrnkbqrb/pppppppp/8/8/8/8/PPPPPPPP/NRNKBQRB w BGbg - 0 1",
"qbbnrkrn/pppppppp/8/8/8/8/PPPPPPPP/QBBNRKRN w EGeg - 0 1",
"nbrnkqbr/pppppppp/8/8/8/8/PPPPPPPP/NBRNKQBR w CHch - 0 1",
"rknrnqbb/pppppppp/8/8/8/8/PPPPPPPP/RKNRNQBB w ADad - 0 1",
"rqnnkrbb/pppppppp/8/8/8/8/PPPPPPPP/RQNNKRBB w AFaf - 0 1",
"nbrqknbr/pppppppp/8/8/8/8/PPPPPPPP/NBRQKNBR w CHch - 0 1",
"nbqrbnkr/pppppppp/8/8/8/8/PPPPPPPP/NBQRBNKR w DHdh - 0 1",
"rqnnbkrb/pppppppp/8/8/8/8/PPPPPPPP/RQNNBKRB w AGag - 0 1",
"rkrnqnbb/pppppppp/8/8/8/8/PPPPPPPP/RKRNQNBB w ACac - 0 1",
"nbbrqnkr/pppppppp/8/8/8/8/PPPPPPPP/NBBRQNKR w DHdh - 0 1",
"nnqrbbkr/pppppppp/8/8/8/8/PPPPPPPP/NNQRBBKR w DHdh - 0 1",
"rnbbqnkr/pppppppp/8/8/8/8/PPPPPPPP/RNBBQNKR w AHah - 0 1",
"qrnkbrnb/pppppppp/8/8/8/8/PPPPPPPP/QRNKBRNB w BFbf - 0 1",
"nbqrbkrn/pppppppp/8/8/8/8/PPPPPPPP/NBQRBKRN w DGdg - 0 1",
"brknnrqb/pppppppp/8/8/8/8/PPPPPPPP/BRKNNRQB w BFbf - 0 1",
"bbrqknrn/pppppppp/8/8/8/8/PPPPPPPP/BBRQKNRN w CGcg - 0 1",
"rkqrbnnb/pppppppp/8/8/8/8/PPPPPPPP/RKQRBNNB w ADad - 0 1",
"nbnrbqkr/pppppppp/8/8/8/8/PPPPPPPP/NBNRBQKR w DHdh - 0 1",
"nnqrkrbb/pppppppp/8/8/8/8/PPPPPPPP/NNQRKRBB w DFdf - 0 1",
"rkbnnbqr/pppppppp/8/8/8/8/PPPPPPPP/RKBNNBQR w AHah - 0 1",
"rkrqnbbn/pppppppp/8/8/8/8/PPPPPPPP/RKRQNBBN w ACac - 0 1",
"nnrkqbbr/pppppppp/8/8/8/8/PPPPPPPP/NNRKQBBR w CHch - 0 1",
"bbrknrqn/pppppppp/8/8/8/8/PPPPPPPP/BBRKNRQN w CFcf - 0 1",
"nrkbnrbq/pppppppp/8/8/8/8/PPPPPPPP/NRKBNRBQ w BFbf - 0 1",
"qrknbbrn/pppppppp/8/8/8/8/PPPPPPPP/QRKNBBRN w BGbg - 0 1",
"rnbknbqr/pppppppp/8/8/8/8/PPPPPPPP/RNBKNBQR w AHah - 0 1",
"rkrbbnqn/pppppppp/8/8/8/8/PPPPPPPP/RKRBBNQN w ACac - 0 1",
"bnrqkrnb/pppppppp/8/8/8/8/PPPPPPPP/BNRQKRNB w CFcf - 0 1",
"bnrqkbnr/pppppppp/8/8/8/8/PPPPPPPP/BNRQKBNR w CHch - 0 1",
"rbbqnnkr/pppppppp/8/8/8/8/PPPPPPPP/RBBQNNKR w AHah - 0 1",
"rkrbnnbq/pppppppp/8/8/8/8/PPPPPPPP/RKRBNNBQ w ACac - 0 1",
"bbrnkqrn/pppppppp/8/8/8/8/PPPPPPPP/BBRNKQRN w CGcg - 0 1",
"nqrknbbr/pppppppp/8/8/8/8/PPPPPPPP/NQRKNBBR w CHch - 0 1",
"nqnrkrbb/pppppppp/8/8/8/8/PPPPPPPP/NQNRKRBB w DFdf - 0 1",
"nrkqrnbb/pppppppp/8/8/8/8/PPPPPPPP/NRKQRNBB w BEbe - 0 1",
"bbrknqrn/pppppppp/8/8/8/8/PPPPPPPP/BBRKNQRN w CGcg - 0 1",
"rbknnqbr/pppppppp/8/8/8/8/PPPPPPPP/RBKNNQBR w AHah - 0 1",
"nqrnkbbr/pppppppp/8/8/8/8/PPPPPPPP/NQRNKBBR w CHch - 0 1",
"nbrqnkbr/pppppppp/8/8/8/8/PPPPPPPP/NBRQNKBR w CHch - 0 1",
"rbbkqnnr/pppppppp/8/8/8/8/PPPPPPPP/RBBKQNNR w AHah - 0 1",
"nrqbknbr/pppppppp/8/8/8/8/PPPPPPPP/NRQBKNBR w BHbh - 0 1",
"rnnkqrbb/pppppppp/8/8/8/8/PPPPPPPP/RNNKQRBB w AFaf - 0 1",
"bnrknqrb/pppppppp/8/8/8/8/PPPPPPPP/BNRKNQRB w CGcg - 0 1",
"rbbqnkrn/pppppppp/8/8/8/8/PPPPPPPP/RBBQNKRN w AGag - 0 1",
"rnknbbqr/pppppppp/8/8/8/8/PPPPPPPP/RNKNBBQR w AHah - 0 1",
"nbrqbknr/pppppppp/8/8/8/8/PPPPPPPP/NBRQBKNR w CHch - 0 1",
"bnrkrbqn/pppppppp/8/8/8/8/PPPPPPPP/BNRKRBQN w CEce - 0 1",
"nrknrqbb/pppppppp/8/8/8/8/PPPPPPPP/NRKNRQBB w BEbe - 0 1",
"rqbnkbrn/pppppppp/8/8/8/8/PPPPPPPP/RQBNKBRN w AGag - 0 1",
"bbqnrkrn/pppppppp/8/8/8/8/PPPPPPPP/BBQNRKRN w EGeg - 0 1",
"rnkqnbbr/pppppppp/8/8/8/8/PPPPPPPP/RNKQNBBR w AHah - 0 1",
"rnqkrbbn/pppppppp/8/8/8/8/PPPPPPPP/RNQKRBBN w AEae - 0 1",
"rknbbqnr/pppppppp/8/8/8/8/PPPPPPPP/RKNBBQNR w AHah - 0 1",
"nrnbbkqr/pppppppp/8/8/8/8/PPPPPPPP/NRNBBKQR w BHbh - 0 1",
"bqnrknrb/pppppppp/8/8/8/8/PPPPPPPP/BQNRKNRB w DGdg - 0 1",
"rkbrnbnq/pppppppp/8/8/8/8/PPPPPPPP/RKBRNBNQ w ADad - 0 1",
"qbbrknrn/pppppppp/8/8/8/8/PPPPPPPP/QBBRKNRN w DGdg - 0 1",
"qrkrnnbb/pppppppp/8/8/8/8/PPPPPPPP/QRKRNNBB w BDbd - 0 1",
"brkrnqnb/pppppppp/8/8/8/8/PPPPPPPP/BRKRNQNB w BDbd - 0 1",
"rqbknbnr/pppppppp/8/8/8/8/PPPPPPPP/RQBKNBNR w AHah - 0 1",
"brnbknrq/pppppppp/8/8/8/8/PPPPPPPP/BRNBKNRQ w BGbg - 0 1",
"rbkqbnrn/pppppppp/8/8/8/8/PPPPPPPP/RBKQBNRN w AGag - 0 1",
"rknnqrbb/pppppppp/8/8/8/8/PPPPPPPP/RKNNQRBB w AFaf - 0 1",
"qrknbnrb/pppppppp/8/8/8/8/PPPPPPPP/QRKNBNRB w BGbg - 0 1",
"qbbrnkrn/pppppppp/8/8/8/8/PPPPPPPP/QBBRNKRN w DGdg - 0 1",
"bbrknnqr/pppppppp/8/8/8/8/PPPPPPPP/BBRKNNQR w CHch - 0 1",
"qnrnkrbb/pppppppp/8/8/8/8/PPPPPPPP/QNRNKRBB w CFcf - 0 1",
"nrbbqknr/pppppppp/8/8/8/8/PPPPPPPP/NRBBQKNR w BHbh - 0 1",
"nbrknrbq/pppppppp/8/8/8/8/PPPPPPPP/NBRKNRBQ w CFcf - 0 1",
"brkqnnrb/pppppppp/8/8/8/8/PPPPPPPP/BRKQNNRB w BGbg - 0 1",
"qnrbbkrn/pppppppp/8/8/8/8/PPPPPPPP/QNRBBKRN w CGcg - 0 1",
"nbbrkrnq/pppppppp/8/8/8/8/PPPPPPPP/NBBRKRNQ w DFdf - 0 1",
"nqrkrnbb/pppppppp/8/8/8/8/PPPPPPPP/NQRKRNBB w CEce - 0 1",
"qnbbrnkr/pppppppp/8/8/8/8/PPPPPPPP/QNBBRNKR w EHeh - 0 1",
"rknbbnqr/pppppppp/8/8/8/8/PPPPPPPP/RKNBBNQR w AHah - 0 1",
"brkbrqnn/pppppppp/8/8/8/8/PPPPPPPP/BRKBRQNN w BEbe - 0 1",
"nbbnrkrq/pppppppp/8/8/8/8/PPPPPPPP/NBBNRKRQ w EGeg - 0 1",
"nrnbkrbq/pppppppp/8/8/8/8/PPPPPPPP/NRNBKRBQ w BFbf - 0 1",
"rknrbnqb/pppppppp/8/8/8/8/PPPPPPPP/RKNRBNQB w ADad - 0 1",
"rbbnknrq/pppppppp/8/8/8/8/PPPPPPPP/RBBNKNRQ w AGag - 0 1",
"rknbbrqn/pppppppp/8/8/8/8/PPPPPPPP/RKNBBRQN w AFaf - 0 1",
"rnnkrqbb/pppppppp/8/8/8/8/PPPPPPPP/RNNKRQBB w AEae - 0 1",
"rnkbbrqn/pppppppp/8/8/8/8/PPPPPPPP/RNKBBRQN w AFaf - 0 1",
"nqbbrnkr/pppppppp/8/8/8/8/PPPPPPPP/NQBBRNKR w EHeh - 0 1",
"rbbknrnq/pppppppp/8/8/8/8/PPPPPPPP/RBBKNRNQ w AFaf - 0 1",
"rqnkbnrb/pppppppp/8/8/8/8/PPPPPPPP/RQNKBNRB w AGag - 0 1",
"qrbbknnr/pppppppp/8/8/8/8/PPPPPPPP/QRBBKNNR w BHbh - 0 1",
"rbqnbkrn/pppppppp/8/8/8/8/PPPPPPPP/RBQNBKRN w AGag - 0 1",
"qrbknnrb/pppppppp/8/8/8/8/PPPPPPPP/QRBKNNRB w BGbg - 0 1",
"brnknbrq/pppppppp/8/8/8/8/PPPPPPPP/BRNKNBRQ w BGbg - 0 1",
"rnbnkqrb/pppppppp/8/8/8/8/PPPPPPPP/RNBNKQRB w AGag - 0 1",
"brnbkqrn/pppppppp/8/8/8/8/PPPPPPPP/BRNBKQRN w BGbg - 0 1",
"rkbnqbnr/pppppppp/8/8/8/8/PPPPPPPP/RKBNQBNR w AHah - 0 1",
"rbqnbnkr/pppppppp/8/8/8/8/PPPPPPPP/RBQNBNKR w AHah - 0 1",
"nbqnbrkr/pppppppp/8/8/8/8/PPPPPPPP/NBQNBRKR w FHfh - 0 1",
"nnbrkbrq/pppppppp/8/8/8/8/PPPPPPPP/NNBRKBRQ w DGdg - 0 1",
"nbbnrqkr/pppppppp/8/8/8/8/PPPPPPPP/NBBNRQKR w EHeh - 0 1",
"rkqnbnrb/pppppppp/8/8/8/8/PPPPPPPP/RKQNBNRB w AGag - 0 1",
"rkbqrbnn/pppppppp/8/8/8/8/PPPPPPPP/RKBQRBNN w AEae - 0 1",
"nrbbqkrn/pppppppp/8/8/8/8/PPPPPPPP/NRBBQKRN w BGbg - 0 1",
"rkqrbbnn/pppppppp/8/8/8/8/PPPPPPPP/RKQRBBNN w ADad - 0 1",
"nrkqbrnb/pppppppp/8/8/8/8/PPPPPPPP/NRKQBRNB w BFbf - 0 1",
"rqbkrbnn/pppppppp/8/8/8/8/PPPPPPPP/RQBKRBNN w AEae - 0 1",
"bbqrnknr/pppppppp/8/8/8/8/PPPPPPPP/BBQRNKNR w DHdh - 0 1",
"brnkqbrn/pppppppp/8/8/8/8/PPPPPPPP/BRNKQBRN w BGbg - 0 1",
"qrbnnbkr/pppppppp/8/8/8/8/PPPPPPPP/QRBNNBKR w BHbh - 0 1",
"nrkrbbnq/pppppppp/8/8/8/8/PPPPPPPP/NRKRBBNQ w BDbd - 0 1",
"qbrnbkrn/pppppppp/8/8/8/8/PPPPPPPP/QBRNBKRN w CGcg - 0 1",
"rnkqrbbn/pppppppp/8/8/8/8/PPPPPPPP/RNKQRBBN w AEae - 0 1",
"rbbqknrn/pppppppp/8/8/8/8/PPPPPPPP/RBBQKNRN w AGag - 0 1",
"rbkrnnbq/pppppppp/8/8/8/8/PPPPPPPP/RBKRNNBQ w ADad - 0 1",
"rkqnbrnb/pppppppp/8/8/8/8/PPPPPPPP/RKQNBRNB w AFaf - 0 1",
"qrnkrnbb/pppppppp/8/8/8/8/PPPPPPPP/QRNKRNBB w BEbe - 0 1",
"rbnkrqbn/pppppppp/8/8/8/8/PPPPPPPP/RBNKRQBN w AEae - 0 1",
"bqrnknrb/pppppppp/8/8/8/8/PPPPPPPP/BQRNKNRB w CGcg - 0 1",
"rnkrqnbb/pppppppp/8/8/8/8/PPPPPPPP/RNKRQNBB w ADad - 0 1",
"nrqbbkrn/pppppppp/8/8/8/8/PPPPPPPP/NRQBBKRN w BGbg - 0 1",
"bbnrqnkr/pppppppp/8/8/8/8/PPPPPPPP/BBNRQNKR w DHdh - 0 1",
"nnrqbbkr/pppppppp/8/8/8/8/PPPPPPPP/NNRQBBKR w CHch - 0 1",
"rknbnqbr/pppppppp/8/8/8/8/PPPPPPPP/RKNBNQBR w AHah - 0 1",
"bbrqknnr/pppppppp/8/8/8/8/PPPPPPPP/BBRQKNNR w CHch - 0 1",
"qnbbrkrn/pppppppp/8/8/8/8/PPPPPPPP/QNBBRKRN w EGeg - 0 1",
"bbnrqkrn/pppppppp/8/8/8/8/PPPPPPPP/BBNRQKRN w DGdg - 0 1",
"qrbnkrnb/pppppppp/8/8/8/8/PPPPPPPP/QRBNKRNB w BFbf - 0 1",
"bqnnrbkr/pppppppp/8/8/8/8/PPPPPPPP/BQNNRBKR w EHeh - 0 1",
"nqbrnkrb/pppppppp/8/8/8/8/PPPPPPPP/NQBRNKRB w DGdg - 0 1",
"rbbnqknr/pppppppp/8/8/8/8/PPPPPPPP/RBBNQKNR w AHah - 0 1",
"bnrnkqrb/pppppppp/8/8/8/8/PPPPPPPP/BNRNKQRB w CGcg - 0 1",
"nrbbkqrn/pppppppp/8/8/8/8/PPPPPPPP/NRBBKQRN w BGbg - 0 1",
"brnnqbkr/pppppppp/8/8/8/8/PPPPPPPP/BRNNQBKR w BHbh - 0 1",
"rnbqnbkr/pppppppp/8/8/8/8/PPPPPPPP/RNBQNBKR w AHah - 0 1",
"rknnbbrq/pppppppp/8/8/8/8/PPPPPPPP/RKNNBBRQ w AGag - 0 1",
"rkbnnbrq/pppppppp/8/8/8/8/PPPPPPPP/RKBNNBRQ w AGag - 0 1",
"rnbnkrqb/pppppppp/8/8/8/8/PPPPPPPP/RNBNKRQB w AFaf - 0 1",
"nnbrqkrb/pppppppp/8/8/8/8/PPPPPPPP/NNBRQKRB w DGdg - 0 1",
"nrkqbbnr/pppppppp/8/8/8/8/PPPPPPPP/NRKQBBNR w BHbh - 0 1",
"brkbqnnr/pppppppp/8/8/8/8/PPPPPPPP/BRKBQNNR w BHbh - 0 1",
"rnkrqbbn/pppppppp/8/8/8/8/PPPPPPPP/RNKRQBBN w ADad - 0 1",
"nrbbnkqr/pppppppp/8/8/8/8/PPPPPPPP/NRBBNKQR w BHbh - 0 1",
"qnrbknbr/pppppppp/8/8/8/8/PPPPPPPP/QNRBKNBR w CHch - 0 1",
"rqnbbnkr/pppppppp/8/8/8/8/PPPPPPPP/RQNBBNKR w AHah - 0 1",
"rknbrqbn/pppppppp/8/8/8/8/PPPPPPPP/RKNBRQBN w AEae - 0 1",
"rkqnbbrn/pppppppp/8/8/8/8/PPPPPPPP/RKQNBBRN w AGag - 0 1",
"rbknnrbq/pppppppp/8/8/8/8/PPPPPPPP/RBKNNRBQ w AFaf - 0 1",
"qnbrkbnr/pppppppp/8/8/8/8/PPPPPPPP/QNBRKBNR w DHdh - 0 1",
"nrbbkrnq/pppppppp/8/8/8/8/PPPPPPPP/NRBBKRNQ w BFbf - 0 1",
"rnknrbbq/pppppppp/8/8/8/8/PPPPPPPP/RNKNRBBQ w AEae - 0 1",
"rkrnqbbn/pppppppp/8/8/8/8/PPPPPPPP/RKRNQBBN w ACac - 0 1",
"rbbkrnqn/pppppppp/8/8/8/8/PPPPPPPP/RBBKRNQN w AEae - 0 1",
"brkrqbnn/pppppppp/8/8/8/8/PPPPPPPP/BRKRQBNN w BDbd - 0 1",
"nbrkbrnq/pppppppp/8/8/8/8/PPPPPPPP/NBRKBRNQ w CFcf - 0 1",
"rqknbbrn/pppppppp/8/8/8/8/PPPPPPPP/RQKNBBRN w AGag - 0 1",
"nrkbqnbr/pppppppp/8/8/8/8/PPPPPPPP/NRKBQNBR w BHbh - 0 1",
"brknqbrn/pppppppp/8/8/8/8/PPPPPPPP/BRKNQBRN w BGbg - 0 1",
"rkbrqbnn/pppppppp/8/8/8/8/PPPPPPPP/RKBRQBNN w ADad - 0 1",
"nrqkrnbb/pppppppp/8/8/8/8/PPPPPPPP/NRQKRNBB w BEbe - 0 1",
"rnkbbqrn/pppppppp/8/8/8/8/PPPPPPPP/RNKBBQRN w AGag - 0 1",
"rkrnbqnb/pppppppp/8/8/8/8/PPPPPPPP/RKRNBQNB w ACac - 0 1",
"bbnrkqrn/pppppppp/8/8/8/8/PPPPPPPP/BBNRKQRN w DGdg - 0 1",
"rqnkrnbb/pppppppp/8/8/8/8/PPPPPPPP/RQNKRNBB w AEae - 0 1",
"brqknbnr/pppppppp/8/8/8/8/PPPPPPPP/BRQKNBNR w BHbh - 0 1",
"bqnrkbnr/pppppppp/8/8/8/8/PPPPPPPP/BQNRKBNR w DHdh - 0 1",
"nqrkbbnr/pppppppp/8/8/8/8/PPPPPPPP/NQRKBBNR w CHch - 0 1",
"nrnbkqbr/pppppppp/8/8/8/8/PPPPPPPP/NRNBKQBR w BHbh - 0 1",
"rqbbknnr/pppppppp/8/8/8/8/PPPPPPPP/RQBBKNNR w AHah - 0 1",
"nrknbqrb/pppppppp/8/8/8/8/PPPPPPPP/NRKNBQRB w BGbg - 0 1",
"nbbnqrkr/pppppppp/8/8/8/8/PPPPPPPP/NBBNQRKR w FHfh - 0 1",
"bnrkqbnr/pppppppp/8/8/8/8/PPPPPPPP/BNRKQBNR w CHch - 0 1",
"nnqbrkbr/pppppppp/8/8/8/8/PPPPPPPP/NNQBRKBR w EHeh - 0 1",
"rbknqnbr/pppppppp/8/8/8/8/PPPPPPPP/RBKNQNBR w AHah - 0 1",
"rkrqbnnb/pppppppp/8/8/8/8/PPPPPPPP/RKRQBNNB w ACac - 0 1",
"rnbnkbqr/pppppppp/8/8/8/8/PPPPPPPP/RNBNKBQR w AHah - 0 1",
"nnbbrkqr/pppppppp/8/8/8/8/PPPPPPPP/NNBBRKQR w EHeh - 0 1",
"rknrqbbn/pppppppp/8/8/8/8/PPPPPPPP/RKNRQBBN w ADad - 0 1",
"rknqbnrb/pppppppp/8/8/8/8/PPPPPPPP/RKNQBNRB w AGag - 0 1",
"rqkrbnnb/pppppppp/8/8/8/8/PPPPPPPP/RQKRBNNB w ADad - 0 1",
"rkbqnbrn/pppppppp/8/8/8/8/PPPPPPPP/RKBQNBRN w AGag - 0 1",
"bnrnkbrq/pppppppp/8/8/8/8/PPPPPPPP/BNRNKBRQ w CGcg - 0 1",
"qbnnrkbr/pppppppp/8/8/8/8/PPPPPPPP/QBNNRKBR w EHeh - 0 1",
"nrbknbqr/pppppppp/8/8/8/8/PPPPPPPP/NRBKNBQR w BHbh - 0 1",
"nrnbbkrq/pppppppp/8/8/8/8/PPPPPPPP/NRNBBKRQ w BGbg - 0 1",
"brnbqnkr/pppppppp/8/8/8/8/PPPPPPPP/BRNBQNKR w BHbh - 0 1",
"rqknbrnb/pppppppp/8/8/8/8/PPPPPPPP/RQKNBRNB w AFaf - 0 1",
"qnnrkbbr/pppppppp/8/8/8/8/PPPPPPPP/QNNRKBBR w DHdh - 0 1",
"bnnrkrqb/pppppppp/8/8/8/8/PPPPPPPP/BNNRKRQB w DFdf - 0 1",
"rbnkbqnr/pppppppp/8/8/8/8/PPPPPPPP/RBNKBQNR w AHah - 0 1",
"brnkqbnr/pppppppp/8/8/8/8/PPPPPPPP/BRNKQBNR w BHbh - 0 1",
"nrkrqnbb/pppppppp/8/8/8/8/PPPPPPPP/NRKRQNBB w BDbd - 0 1",
"bqrbknnr/pppppppp/8/8/8/8/PPPPPPPP/BQRBKNNR w CHch - 0 1",
"nbbqrkrn/pppppppp/8/8/8/8/PPPPPPPP/NBBQRKRN w EGeg - 0 1",
"rqbknnrb/pppppppp/8/8/8/8/PPPPPPPP/RQBKNNRB w AGag - 0 1",
"nbbrkrqn/pppppppp/8/8/8/8/PPPPPPPP/NBBRKRQN w DFdf - 0 1",
"nrbbknrq/pppppppp/8/8/8/8/PPPPPPPP/NRBBKNRQ w BGbg - 0 1",
"rqkbnnbr/pppppppp/8/8/8/8/PPPPPPPP/RQKBNNBR w AHah - 0 1",
"nrknqrbb/pppppppp/8/8/8/8/PPPPPPPP/NRKNQRBB w BFbf - 0 1",
"brnnqkrb/pppppppp/8/8/8/8/PPPPPPPP/BRNNQKRB w BGbg - 0 1",
"rqbkrnnb/pppppppp/8/8/8/8/PPPPPPPP/RQBKRNNB w AEae - 0 1",
"rbnnqkbr/pppppppp/8/8/8/8/PPPPPPPP/RBNNQKBR w AHah - 0 1",
"brknnbqr/pppppppp/8/8/8/8/PPPPPPPP/BRKNNBQR w BHbh - 0 1",
"nbbrkqnr/pppppppp/8/8/8/8/PPPPPPPP/NBBRKQNR w DHdh - 0 1",
"bbnqrkrn/pppppppp/8/8/8/8/PPPPPPPP/BBNQRKRN w EGeg - 0 1",
"qbrkbnnr/pppppppp/8/8/8/8/PPPPPPPP/QBRKBNNR w CHch - 0 1",
"nrbnkqrb/pppppppp/8/8/8/8/PPPPPPPP/NRBNKQRB w BGbg - 0 1",
"rbkqnnbr/pppppppp/8/8/8/8/PPPPPPPP/RBKQNNBR w AHah - 0 1",
"qbnrbkrn/pppppppp/8/8/8/8/PPPPPPPP/QBNRBKRN w DGdg - 0 1",
"nqrnbbkr/pppppppp/8/8/8/8/PPPPPPPP/NQRNBBKR w CHch - 0 1",
"brnbkrqn/pppppppp/8/8/8/8/PPPPPPPP/BRNBKRQN w BFbf - 0 1",
"brqbnknr/pppppppp/8/8/8/8/PPPPPPPP/BRQBNKNR w BHbh - 0 1",
"brqbkrnn/pppppppp/8/8/8/8/PPPPPPPP/BRQBKRNN w BFbf - 0 1",
"nqrbkrbn/pppppppp/8/8/8/8/PPPPPPPP/NQRBKRBN w CFcf - 0 1",
"rkbnrnqb/pppppppp/8/8/8/8/PPPPPPPP/RKBNRNQB w AEae - 0 1",
"brnkrqnb/pppppppp/8/8/8/8/PPPPPPPP/BRNKRQNB w BEbe - 0 1",
"brnkqrnb/pppppppp/8/8/8/8/PPPPPPPP/BRNKQRNB w BFbf - 0 1",
"brqnnkrb/pppppppp/8/8/8/8/PPPPPPPP/BRQNNKRB w BGbg - 0 1",
"rbbqkrnn/pppppppp/8/8/8/8/PPPPPPPP/RBBQKRNN w AFaf - 0 1",
"rnkqrnbb/pppppppp/8/8/8/8/PPPPPPPP/RNKQRNBB w AEae - 0 1",
"nbbrnqkr/pppppppp/8/8/8/8/PPPPPPPP/NBBRNQKR w DHdh - 0 1",
"rbqnnkbr/pppppppp/8/8/8/8/PPPPPPPP/RBQNNKBR w AHah - 0 1",
"rbnqbknr/pppppppp/8/8/8/8/PPPPPPPP/RBNQBKNR w AHah - 0 1",
"rbnkbrqn/pppppppp/8/8/8/8/PPPPPPPP/RBNKBRQN w AFaf - 0 1",
"brnknbqr/pppppppp/8/8/8/8/PPPPPPPP/BRNKNBQR w BHbh - 0 1",
"nrknqbbr/pppppppp/8/8/8/8/PPPPPPPP/NRKNQBBR w BHbh - 0 1",
"rbnknrbq/pppppppp/8/8/8/8/PPPPPPPP/RBNKNRBQ w AFaf - 0 1",
"rkbnqrnb/pppppppp/8/8/8/8/PPPPPPPP/RKBNQRNB w AFaf - 0 1",
"nrnkqbbr/pppppppp/8/8/8/8/PPPPPPPP/NRNKQBBR w BHbh - 0 1",
"rkqrnbbn/pppppppp/8/8/8/8/PPPPPPPP/RKQRNBBN w ADad - 0 1",
"bqrbnknr/pppppppp/8/8/8/8/PPPPPPPP/BQRBNKNR w CHch - 0 1",
"bnnrqbkr/pppppppp/8/8/8/8/PPPPPPPP/BNNRQBKR w DHdh - 0 1",
"rkbqnbnr/pppppppp/8/8/8/8/PPPPPPPP/RKBQNBNR w AHah - 0 1",
"bbqnrknr/pppppppp/8/8/8/8/PPPPPPPP/BBQNRKNR w EHeh - 0 1",
"bnrbnkqr/pppppppp/8/8/8/8/PPPPPPPP/BNRBNKQR w CHch - 0 1",
"nqrbbnkr/pppppppp/8/8/8/8/PPPPPPPP/NQRBBNKR w CHch - 0 1",
"bnrqnkrb/pppppppp/8/8/8/8/PPPPPPPP/BNRQNKRB w CGcg - 0 1",
"brkqrnnb/pppppppp/8/8/8/8/PPPPPPPP/BRKQRNNB w BEbe - 0 1",
"nqnbbrkr/pppppppp/8/8/8/8/PPPPPPPP/NQNBBRKR w FHfh - 0 1",
"rnkrbbnq/pppppppp/8/8/8/8/PPPPPPPP/RNKRBBNQ w ADad - 0 1",
"rnkqbnrb/pppppppp/8/8/8/8/PPPPPPPP/RNKQBNRB w AGag - 0 1",
"qrnbnkbr/pppppppp/8/8/8/8/PPPPPPPP/QRNBNKBR w BHbh - 0 1",
"rbkrnqbn/pppppppp/8/8/8/8/PPPPPPPP/RBKRNQBN w ADad - 0 1",
"nqbrknrb/pppppppp/8/8/8/8/PPPPPPPP/NQBRKNRB w DGdg - 0 1",
"qrkbnnbr/pppppppp/8/8/8/8/PPPPPPPP/QRKBNNBR w BHbh - 0 1",
"qrnkrbbn/pppppppp/8/8/8/8/PPPPPPPP/QRNKRBBN w BEbe - 0 1",
"bqnbrkrn/pppppppp/8/8/8/8/PPPPPPPP/BQNBRKRN w EGeg - 0 1",
"nqbbrknr/pppppppp/8/8/8/8/PPPPPPPP/NQBBRKNR w EHeh - 0 1",
"nrbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/NRBQKBNR w BHbh - 0 1",
"rnqknbbr/pppppppp/8/8/8/8/PPPPPPPP/RNQKNBBR w AHah - 0 1",
"nnrkqrbb/pppppppp/8/8/8/8/PPPPPPPP/NNRKQRBB w CFcf - 0 1",
"qnrbbknr/pppppppp/8/8/8/8/PPPPPPPP/QNRBBKNR w CHch - 0 1",
"bbnnrkqr/pppppppp/8/8/8/8/PPPPPPPP/BBNNRKQR w EHeh - 0 1",
"rkbqnrnb/pppppppp/8/8/8/8/PPPPPPPP/RKBQNRNB w AFaf - 0 1",
"nbrqbnkr/pppppppp/8/8/8/8/PPPPPPPP/NBRQBNKR w CHch - 0 1",
"nqbnrkrb/pppppppp/8/8/8/8/PPPPPPPP/NQBNRKRB w EGeg - 0 1",
"rnnbbkrq/pppppppp/8/8/8/8/PPPPPPPP/RNNBBKRQ w AGag - 0 1",
"brnqnkrb/pppppppp/8/8/8/8/PPPPPPPP/BRNQNKRB w BGbg - 0 1",
"bnrnkrqb/pppppppp/8/8/8/8/PPPPPPPP/BNRNKRQB w CFcf - 0 1",
"qrbbkrnn/pppppppp/8/8/8/8/PPPPPPPP/QRBBKRNN w BFbf - 0 1",
"qbrnkrbn/pppppppp/8/8/8/8/PPPPPPPP/QBRNKRBN w CFcf - 0 1",
"nrkrqbbn/pppppppp/8/8/8/8/PPPPPPPP/NRKRQBBN w BDbd - 0 1",
"nrkbbrqn/pppppppp/8/8/8/8/PPPPPPPP/NRKBBRQN w BFbf - 0 1",
"qrbkrbnn/pppppppp/8/8/8/8/PPPPPPPP/QRBKRBNN w BEbe - 0 1",
"bnrbqknr/pppppppp/8/8/8/8/PPPPPPPP/BNRBQKNR w CHch - 0 1",
"brkqnbnr/pppppppp/8/8/8/8/PPPPPPPP/BRKQNBNR w BHbh - 0 1",
"rkbbqrnn/pppppppp/8/8/8/8/PPPPPPPP/RKBBQRNN w AFaf - 0 1",
"rknqnbbr/pppppppp/8/8/8/8/PPPPPPPP/RKNQNBBR w AHah - 0 1",
"rnqbbnkr/pppppppp/8/8/8/8/PPPPPPPP/RNQBBNKR w AHah - 0 1",
"rbqnbknr/pppppppp/8/8/8/8/PPPPPPPP/RBQNBKNR w AHah - 0 1",
"brknrbnq/pppppppp/8/8/8/8/PPPPPPPP/BRKNRBNQ w BEbe - 0 1",
"bqrnkbrn/pppppppp/8/8/8/8/PPPPPPPP/BQRNKBRN w CGcg - 0 1",
"nqnrbkrb/pppppppp/8/8/8/8/PPPPPPPP/NQNRBKRB w DGdg - 0 1",
"qrkbbrnn/pppppppp/8/8/8/8/PPPPPPPP/QRKBBRNN w BFbf - 0 1",
"brkqnbrn/pppppppp/8/8/8/8/PPPPPPPP/BRKQNBRN w BGbg - 0 1",
"rqknrnbb/pppppppp/8/8/8/8/PPPPPPPP/RQKNRNBB w AEae - 0 1",
"brqnkbrn/pppppppp/8/8/8/8/PPPPPPPP/BRQNKBRN w BGbg - 0 1",
"brkrnnqb/pppppppp/8/8/8/8/PPPPPPPP/BRKRNNQB w BDbd - 0 1",
"bbnrkqnr/pppppppp/8/8/8/8/PPPPPPPP/BBNRKQNR w DHdh - 0 1",
"brnbnqkr/pppppppp/8/8/8/8/PPPPPPPP/BRNBNQKR w BHbh - 0 1",
"qbnrknbr/pppppppp/8/8/8/8/PPPPPPPP/QBNRKNBR w DHdh - 0 1",
"rnqknrbb/pppppppp/8/8/8/8/PPPPPPPP/RNQKNRBB w AFaf - 0 1",
"rqkbbnnr/pppppppp/8/8/8/8/PPPPPPPP/RQKBBNNR w AHah - 0 1",
"bbqrknrn/pppppppp/8/8/8/8/PPPPPPPP/BBQRKNRN w DGdg - 0 1",
"nbbnrkqr/pppppppp/8/8/8/8/PPPPPPPP/NBBNRKQR w EHeh - 0 1",
"rbknbrqn/pppppppp/8/8/8/8/PPPPPPPP/RBKNBRQN w AFaf - 0 1",
"rnbbknqr/pppppppp/8/8/8/8/PPPPPPPP/RNBBKNQR w AHah - 0 1",
"rqkbbrnn/pppppppp/8/8/8/8/PPPPPPPP/RQKBBRNN w AFaf - 0 1",
"qrnknbbr/pppppppp/8/8/8/8/PPPPPPPP/QRNKNBBR w BHbh - 0 1",
"bqnrnkrb/pppppppp/8/8/8/8/PPPPPPPP/BQNRNKRB w DGdg - 0 1",
"rkqbnnbr/pppppppp/8/8/8/8/PPPPPPPP/RKQBNNBR w AHah - 0 1",
"nbbrkqrn/pppppppp/8/8/8/8/PPPPPPPP/NBBRKQRN w DGdg - 0 1",
"bqrnnbkr/pppppppp/8/8/8/8/PPPPPPPP/BQRNNBKR w CHch - 0 1",
"brqbnkrn/pppppppp/8/8/8/8/PPPPPPPP/BRQBNKRN w BGbg - 0 1",
"nqrkbnrb/pppppppp/8/8/8/8/PPPPPPPP/NQRKBNRB w CGcg - 0 1",
"brkbnrqn/pppppppp/8/8/8/8/PPPPPPPP/BRKBNRQN w BFbf - 0 1",
"rqbnknrb/pppppppp/8/8/8/8/PPPPPPPP/RQBNKNRB w AGag - 0 1",
"nrbknbrq/pppppppp/8/8/8/8/PPPPPPPP/NRBKNBRQ w BGbg - 0 1",
"rbbkrqnn/pppppppp/8/8/8/8/PPPPPPPP/RBBKRQNN w AEae - 0 1",
"qnbrnbkr/pppppppp/8/8/8/8/PPPPPPPP/QNBRNBKR w DHdh - 0 1",
"nbnrkqbr/pppppppp/8/8/8/8/PPPPPPPP/NBNRKQBR w DHdh - 0 1",
"rbnkrnbq/pppppppp/8/8/8/8/PPPPPPPP/RBNKRNBQ w AEae - 0 1",
"nnrqkrbb/pppppppp/8/8/8/8/PPPPPPPP/NNRQKRBB w CFcf - 0 1",
"nbbrqkrn/pppppppp/8/8/8/8/PPPPPPPP/NBBRQKRN w DGdg - 0 1",
"rknnbqrb/pppppppp/8/8/8/8/PPPPPPPP/RKNNBQRB w AGag - 0 1",
"rbqkbrnn/pppppppp/8/8/8/8/PPPPPPPP/RBQKBRNN w AFaf - 0 1",
"bnrknbqr/pppppppp/8/8/8/8/PPPPPPPP/BNRKNBQR w CHch - 0 1",
"rkbrqnnb/pppppppp/8/8/8/8/PPPPPPPP/RKBRQNNB w ADad - 0 1",
"brqnkrnb/pppppppp/8/8/8/8/PPPPPPPP/BRQNKRNB w BFbf - 0 1",
"nbnrkrbq/pppppppp/8/8/8/8/PPPPPPPP/NBNRKRBQ w DFdf - 0 1",
"nbrkbrqn/pppppppp/8/8/8/8/PPPPPPPP/NBRKBRQN w CFcf - 0 1",
"bnnqrbkr/pppppppp/8/8/8/8/PPPPPPPP/BNNQRBKR w EHeh - 0 1",
"rbnnkqbr/pppppppp/8/8/8/8/PPPPPPPP/RBNNKQBR w AHah - 0 1",
"qrnbknbr/pppppppp/8/8/8/8/PPPPPPPP/QRNBKNBR w BHbh - 0 1",
"rkqnbbnr/pppppppp/8/8/8/8/PPPPPPPP/RKQNBBNR w AHah - 0 1",
"rnknqbbr/pppppppp/8/8/8/8/PPPPPPPP/RNKNQBBR w AHah - 0 1",
"rknrbbnq/pppppppp/8/8/8/8/PPPPPPPP/RKNRBBNQ w ADad - 0 1",
"nnrkbqrb/pppppppp/8/8/8/8/PPPPPPPP/NNRKBQRB w CGcg - 0 1",
"rnbbnqkr/pppppppp/8/8/8/8/PPPPPPPP/RNBBNQKR w AHah - 0 1",
"bnnrqkrb/pppppppp/8/8/8/8/PPPPPPPP/BNNRQKRB w DGdg - 0 1",
"rbbqknnr/pppppppp/8/8/8/8/PPPPPPPP/RBBQKNNR w AHah - 0 1",
"rkrbnqbn/pppppppp/8/8/8/8/PPPPPPPP/RKRBNQBN w ACac - 0 1",
"brqbnnkr/pppppppp/8/8/8/8/PPPPPPPP/BRQBNNKR w BHbh - 0 1",
"qbrnnkbr/pppppppp/8/8/8/8/PPPPPPPP/QBRNNKBR w CHch - 0 1",
"rbbnkqrn/pppppppp/8/8/8/8/PPPPPPPP/RBBNKQRN w AGag - 0 1",
"bbqnnrkr/pppppppp/8/8/8/8/PPPPPPPP/BBQNNRKR w FHfh - 0 1",
"rnknqrbb/pppppppp/8/8/8/8/PPPPPPPP/RNKNQRBB w AFaf - 0 1",
"bnrqnbkr/pppppppp/8/8/8/8/PPPPPPPP/BNRQNBKR w CHch - 0 1",
"rqbnnbkr/pppppppp/8/8/8/8/PPPPPPPP/RQBNNBKR w AHah - 0 1",
"qbnrbknr/pppppppp/8/8/8/8/PPPPPPPP/QBNRBKNR w DHdh - 0 1",
"qrknrbbn/pppppppp/8/8/8/8/PPPPPPPP/QRKNRBBN w BEbe - 0 1",
"rkrnnqbb/pppppppp/8/8/8/8/PPPPPPPP/RKRNNQBB w ACac - 0 1",
"bbnnrkrq/pppppppp/8/8/8/8/PPPPPPPP/BBNNRKRQ w EGeg - 0 1",
"bnrnkbqr/pppppppp/8/8/8/8/PPPPPPPP/BNRNKBQR w CHch - 0 1",
"rbqkrnbn/pppppppp/8/8/8/8/PPPPPPPP/RBQKRNBN w AEae - 0 1",
"nrqnbbkr/pppppppp/8/8/8/8/PPPPPPPP/NRQNBBKR w BHbh - 0 1",
"rkbbnnrq/pppppppp/8/8/8/8/PPPPPPPP/RKBBNNRQ w AGag - 0 1",
"bnnbrkrq/pppppppp/8/8/8/8/PPPPPPPP/BNNBRKRQ w EGeg - 0 1",
"rbkrbnnq/pppppppp/8/8/8/8/PPPPPPPP/RBKRBNNQ w ADad - 0 1",
"rbqnknbr/pppppppp/8/8/8/8/PPPPPPPP/RBQNKNBR w AHah - 0 1",
"bnrnqkrb/pppppppp/8/8/8/8/PPPPPPPP/BNRNQKRB w CGcg - 0 1",
"rnqkbrnb/pppppppp/8/8/8/8/PPPPPPPP/RNQKBRNB w AFaf - 0 1",
"rnkbrnbq/pppppppp/8/8/8/8/PPPPPPPP/RNKBRNBQ w AEae - 0 1",
"rnbkqrnb/pppppppp/8/8/8/8/PPPPPPPP/RNBKQRNB w AFaf - 0 1",
"rnnkrbbq/pppppppp/8/8/8/8/PPPPPPPP/RNNKRBBQ w AEae - 0 1",
"qbnrbnkr/pppppppp/8/8/8/8/PPPPPPPP/QBNRBNKR w DHdh - 0 1",
"rnqnkbbr/pppppppp/8/8/8/8/PPPPPPPP/RNQNKBBR w AHah - 0 1",
"rkqbnrbn/pppppppp/8/8/8/8/PPPPPPPP/RKQBNRBN w AFaf - 0 1",
"nnrbkrbq/pppppppp/8/8/8/8/PPPPPPPP/NNRBKRBQ w CFcf - 0 1",
"bnqbnrkr/pppppppp/8/8/8/8/PPPPPPPP/BNQBNRKR w FHfh - 0 1",
"nrnqkbbr/pppppppp/8/8/8/8/PPPPPPPP/NRNQKBBR w BHbh - 0 1",
"nrbnkbqr/pppppppp/8/8/8/8/PPPPPPPP/NRBNKBQR w BHbh - 0 1",
"nbbrknrq/pppppppp/8/8/8/8/PPPPPPPP/NBBRKNRQ w DGdg - 0 1",
"brnkrbqn/pppppppp/8/8/8/8/PPPPPPPP/BRNKRBQN w BEbe - 0 1",
"nrnqbkrb/pppppppp/8/8/8/8/PPPPPPPP/NRNQBKRB w BGbg - 0 1",
"rbknbnqr/pppppppp/8/8/8/8/PPPPPPPP/RBKNBNQR w AHah - 0 1",
"rnkbbnrq/pppppppp/8/8/8/8/PPPPPPPP/RNKBBNRQ w AGag - 0 1",
"rnkrbnqb/pppppppp/8/8/8/8/PPPPPPPP/RNKRBNQB w ADad - 0 1",
"nrbbnkrq/pppppppp/8/8/8/8/PPPPPPPP/NRBBNKRQ w BGbg - 0 1",
"bnrkqbrn/pppppppp/8/8/8/8/PPPPPPPP/BNRKQBRN w CGcg - 0 1",
"nbrnkrbq/pppppppp/8/8/8/8/PPPPPPPP/NBRNKRBQ w CFcf - 0 1",
"nrqkbbnr/pppppppp/8/8/8/8/PPPPPPPP/NRQKBBNR w BHbh - 0 1",
"rqnbbkrn/pppppppp/8/8/8/8/PPPPPPPP/RQNBBKRN w AGag - 0 1",
"rbbnknqr/pppppppp/8/8/8/8/PPPPPPPP/RBBNKNQR w AHah - 0 1",
"nrqbnkbr/pppppppp/8/8/8/8/PPPPPPPP/NRQBNKBR w BHbh - 0 1",
"qrnkbbrn/pppppppp/8/8/8/8/PPPPPPPP/QRNKBBRN w BGbg - 0 1",
"rkbqrnnb/pppppppp/8/8/8/8/PPPPPPPP/RKBQRNNB w AEae - 0 1",
"brnkqnrb/pppppppp/8/8/8/8/PPPPPPPP/BRNKQNRB w BGbg - 0 1"
};
// variables

static const bool Strict = FALSE;

// macros

#define skip_white_space() \
        c=string[pos];\
        if (c != ' ' && c!='\t') my_fatal("board_from_fen(): bad FEN (pos=%d)\n",pos); \
        while(c==' ' || c=='\t') c=string[++pos];


// functions

// board_from_fen()

bool board_from_fen(board_t * board, const char string[]) {

   int pos;
   int file, rank, sq;
   int c;
   int i, len;
   int piece;
   int king_pos[ColourNb];

   ASSERT(board!=NULL);
   ASSERT(string!=NULL);

   board_clear(board);

   king_pos[White] = SquareNone;
   king_pos[Black] = SquareNone;

   pos = 0;
   c = string[pos];

   // piece placement

   for (rank = 7; rank >= 0; rank--) {

      for (file = 0; file < 8;) {

         sq = square_make(file,rank);

         if (c >= '1' && c <= '8') { // empty square(s)

            len = c - '0';
            if (file + len > 8) my_fatal("board_from_fen(): bad FEN (pos=%d)\n",pos);

            for (i = 0; i < len; i++) {
               board->square[sq++] = Empty;
               file++;
            }

         } else { // piece

            piece = piece_from_char(c);
            if (piece == PieceNone256) my_fatal("board_from_fen(): bad FEN (pos=%d)\n",pos);

            if (piece_is_king(piece)) king_pos[piece_colour(piece)] = sq;

            board->square[sq++] = piece;
            file++;
         }

         c = string[++pos];
      }

      if (rank > 0) {
         if (c != '/') my_fatal("board_from_fen(): bad FEN (pos=%d)\n",pos);
         c = string[++pos];
     }
   }

   // active colour

   skip_white_space();

   switch (c) {
   case 'w':
      board->turn = White;
      break;
   case 'b':
      board->turn = Black;
      break;
   default:
      my_fatal("board_from_fen(): bad FEN (pos=%d)\n",pos);
      break;
   }

   c = string[++pos];

   // castling

   skip_white_space();

   board->castle[White][SideH] = SquareNone;
   board->castle[White][SideA] = SquareNone;
   board->castle[Black][SideH] = SquareNone;
   board->castle[Black][SideA] = SquareNone;

   if (c == '-') { // no castling rights

      c = string[++pos];

   } else {

      // TODO: filter out illegal rights

      do {

         if (FALSE) {

         } else if (c == 'K') {

            for (sq = H1; sq > king_pos[White]; sq--) {
               if (board->square[sq] == WhiteRook256) {
                  board->castle[White][SideH] = sq;
                  break;
               }
            }

         } else if (c == 'Q') {

            for (sq = A1; sq < king_pos[White]; sq++) {
               if (board->square[sq] == WhiteRook256) {
                  board->castle[White][SideA] = sq;
                  break;
               }
            }

         } else if (c == 'k') {

            for (sq = H8; sq > king_pos[Black]; sq--) {
               if (board->square[sq] == BlackRook256) {
                  board->castle[Black][SideH] = sq;
                  break;
               }
            }

         } else if (c == 'q') {

            for (sq = A8; sq < king_pos[Black]; sq++) {
               if (board->square[sq] == BlackRook256) {
                  board->castle[Black][SideA] = sq;
                  break;
               }
            }

         } else if (c >= 'A' && c <= 'H') {

            // white castling right

            sq = square_make(file_from_char(tolower(c)),Rank1);

            if (sq > king_pos[White]) { // h side
               board->castle[White][SideH] = sq;
            } else { // a side
               board->castle[White][SideA] = sq;
            }

         } else if (c >= 'a' && c <= 'h') {

            // black castling right

            sq = square_make(file_from_char(tolower(c)),Rank8);

            if (sq > king_pos[Black]) { // h side
               board->castle[Black][SideH] = sq;
            } else { // a side
               board->castle[Black][SideA] = sq;
            }

         } else {

            my_fatal("board_from_fen(): bad FEN (pos=%d)\n",pos);
         }

         c = string[++pos];

      } while (c != ' ');
   }

   // en-passant

   skip_white_space();

   if (c == '-') { // no en-passant

      sq = SquareNone;
      c = string[++pos];

   } else {

      if (c < 'a' || c > 'h') my_fatal("board_from_fen(): bad FEN (pos=%d)\n",pos);
      file = file_from_char(c);
      c = string[++pos];

      if (c < '1' || c > '8') my_fatal("board_from_fen(): bad FEN (pos=%d)\n",pos);
      rank = rank_from_char(c);
      c = string[++pos];

      sq = square_make(file,rank);
   }

   board->ep_square = sq;

   // halfmove clock

   board->ply_nb = 0;
   board->move_nb = 0; // HACK, in case of broken syntax

   if (c != ' ') {
      if (!Strict) goto update;
      my_fatal("board_from_fen(): bad FEN (pos=%d)\n",pos);
   }
   c = string[++pos];

   if (!isdigit(c)) {
      if (!Strict) goto update;
      my_fatal("board_from_fen(): bad FEN (pos=%d)\n",pos);
   }

   board->ply_nb = atoi(&string[pos]);
   do c = string[++pos]; while (isdigit(c));

   // fullmove number

   board->move_nb = 0;

   if (c != ' ') {
      if (!Strict) goto update;
      my_fatal("board_from_fen(): bad FEN (pos=%d)\n",pos);
   }
   c = string[++pos];

   if (!isdigit(c)) {
      if (!Strict) goto update;
      my_fatal("board_from_fen(): bad FEN (pos=%d)\n",pos);
   }

   board->move_nb = atoi(&string[pos]) - 1;
   do c = string[++pos]; while (isdigit(c));

   // board update

update:
   board_init_list(board);

   return TRUE;
}

// board_to_fen()

bool board_to_fen(const board_t * board, char string[], int size) {

   int pos;
   int file, rank;
   int sq, piece;
   int c;
   int len;
   int old_pos;

   ASSERT(board_is_ok(board));
   ASSERT(string!=NULL);
   ASSERT(size>=92);

   // init

   if (size < 92) return FALSE;

   pos = 0;

   // piece placement

   for (rank = 7; rank >= 0; rank--) {

      for (file = 0; file < 8;) {

         sq = square_make(file,rank);
         piece = board->square[sq];
         ASSERT(piece==Empty||piece_is_ok(piece));

         if (piece == Empty) {

            len = 0;
            for (; file < 8 && board->square[square_make(file,rank)] == Empty; file++) {
               len++;
            }

            ASSERT(len>=1&&len<=8);
            c = '0' + len;

         } else {

            c = piece_to_char(piece);
            file++;
         }

         string[pos++] = c;
      }

      string[pos++] = '/';
   }

   string[pos-1] = ' '; // HACK: remove the last '/'

   // active colour

   string[pos++] = (colour_is_white(board->turn)) ? 'w' : 'b';
   string[pos++] = ' ';

   // castling

   old_pos = pos;

   bool chess960 = option_get_bool(Option,"Chess960");
   if (!chess960) {
      // Chess960 castling rules: switch to FEN-960 notation

      if (board->castle[White][SideA] != SquareNone) chess960 = chess960 || board->castle[White][SideA] != A1 || king_pos(board,White) != E1;
      if (board->castle[White][SideH] != SquareNone) chess960 = chess960 || board->castle[White][SideH] != H1 || king_pos(board,White) != E1;
      if (board->castle[Black][SideA] != SquareNone) chess960 = chess960 || board->castle[Black][SideA] != A8 || king_pos(board,Black) != E8;
      if (board->castle[Black][SideH] != SquareNone) chess960 = chess960 || board->castle[Black][SideH] != H8 || king_pos(board,Black) != E8;
   }
   if (chess960) {

      // FEN-960

      if (board->castle[White][SideH] != SquareNone) {
         string[pos++] = toupper(file_to_char(square_file(board->castle[White][SideH])));
      }

      if (board->castle[White][SideA] != SquareNone) {
         string[pos++] = toupper(file_to_char(square_file(board->castle[White][SideA])));
      }

      if (board->castle[Black][SideH] != SquareNone) {
         string[pos++] = tolower(file_to_char(square_file(board->castle[Black][SideH])));
      }

      if (board->castle[Black][SideA] != SquareNone) {
         string[pos++] = tolower(file_to_char(square_file(board->castle[Black][SideA])));
      }

   } else {

      // FEN

      if (board->castle[White][SideH] != SquareNone) string[pos++] = 'K';
      if (board->castle[White][SideA] != SquareNone) string[pos++] = 'Q';
      if (board->castle[Black][SideH] != SquareNone) string[pos++] = 'k';
      if (board->castle[Black][SideA] != SquareNone) string[pos++] = 'q';
   }

   if (pos == old_pos) string[pos++] = '-';

   string[pos++] = ' ';

   // en-passant

   if (board->ep_square == SquareNone) {
      string[pos++] = '-';
   } else {
      if (!square_to_string(board->ep_square,&string[pos],3)) return FALSE;
      pos += 2;
   }

   string[pos++] = ' ';

   // halfmove clock and fullmove number

   sprintf(&string[pos],"%d %d",board->ply_nb,board->move_nb+1);

   return TRUE;
}

// end of fen.cpp


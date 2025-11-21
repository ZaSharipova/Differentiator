#ifndef ENUMS_H_
#define ENUMS_H_

#define MAX_VARIABLES 26

enum DifErrors {
    kSuccess,
    kErrorStat,
    kSyntaxError,
    kNoMemory,
    kFailure,
    kErrorOpening,
};

enum DifTypes {
    kOperation,
    kVariable,
    kNumber,
};

enum OperationTypes {
    kAdd,
    kSub,
    kMul,
    kDiv,
    kPow,
    kSin,
    kCos,
    kTg,
    kLn,
    kArctg,
    kNone,
};

#endif //ENUMS_H_
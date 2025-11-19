#ifndef ENUMS_H_
#define ENUMS_H_

#define MAX_VARIABLES 26
#define VARIABLE_NOT_FOUND -1.0/0.0 // NaN

enum DifErrors {
    kSuccess,
    kErrorStat,
    kSyntaxError,
    kNoMemory,
    kFailure,
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
};

#endif //ENUMS_H_
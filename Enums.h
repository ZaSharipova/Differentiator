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
    kOperationAdd, //kOperationAdd
    kOperationSub,
    kOperationMul,
    kOperationDiv,
    kOperationPow,
    kOperationSin,
    kOperationCos,
    kOperationTg,
    kOperationLn,
    kOperationArctg,
    kOperationNone,
};

#endif //ENUMS_H_
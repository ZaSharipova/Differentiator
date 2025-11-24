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
    kOperationAdd,
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

enum DiffModes {
    kDerivative      = 1,
    kDerivativeInPos = 2,
    kCount           = 3,
    kTeilor          = 4,
    kGraph           = 5,
};

#endif //ENUMS_H_
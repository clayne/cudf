# Copyright (c) 2024, NVIDIA CORPORATION.
from enum import IntEnum

from pylibcudf.scalar import Scalar

class TableReference(IntEnum):
    LEFT = ...
    RIGHT = ...

class ASTOperator(IntEnum):
    ADD = ...
    SUB = ...
    MUL = ...
    DIV = ...
    TRUE_DIV = ...
    FLOOR_DIV = ...
    MOD = ...
    PYMOD = ...
    POW = ...
    EQUAL = ...
    NULL_EQUAL = ...
    NOT_EQUAL = ...
    LESS = ...
    GREATER = ...
    LESS_EQUAL = ...
    GREATER_EQUAL = ...
    BITWISE_AND = ...
    BITWISE_OR = ...
    BITWISE_XOR = ...
    NULL_LOGICAL_AND = ...
    LOGICAL_AND = ...
    NULL_LOGICAL_OR = ...
    LOGICAL_OR = ...
    IDENTITY = ...
    IS_NULL = ...
    SIN = ...
    COS = ...
    TAN = ...
    ARCSIN = ...
    ARCCOS = ...
    ARCTAN = ...
    SINH = ...
    COSH = ...
    TANH = ...
    ARCSINH = ...
    ARCCOSH = ...
    ARCTANH = ...
    EXP = ...
    LOG = ...
    SQRT = ...
    CBRT = ...
    CEIL = ...
    FLOOR = ...
    ABS = ...
    RINT = ...
    BIT_INVERT = ...
    NOT = ...

class Expression:
    def __init__(self): ...

class Literal(Expression):
    def __init__(self, value: Scalar): ...

class ColumnReference(Expression):
    def __init__(
        self, index: int, table_source: TableReference = TableReference.LEFT
    ): ...

class ColumnNameReference(Expression):
    def __init__(self, name: str): ...

class Operation(Expression):
    def __init__(
        self,
        op: ASTOperator,
        left: Expression,
        right: Expression | None = None,
    ): ...

def to_expression(expr: str, column_names: tuple[str, ...]) -> Expression: ...
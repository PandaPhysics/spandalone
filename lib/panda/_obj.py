from base import Definition
from oneliner import Include
from constexpr import Constant, Enum
from refbranch import RefBranch
from refvbranch import RefVectorBranch
from generic import GenericBranch
from objbranch import ObjBranch
from branch import Branch
from reference import Reference
from function import Function
from obj import Object

def __init__(self, name, source):
    """
    Constructor called either by PhysicsObject or Tree.
    Parse the source text block and collect all information on this object.
    """

    self.name = name
    self.includes = []
    self.constants = []
    self.enums = []
    self.objbranches = []
    self.branches = []
    self.references = []
    self.functions = []

    while True:
        line = source.readline()
        line = line.strip()

        if line == '':
            break

        try:
            self.includes.append(Include(line))
            continue
        except Definition.NoMatch:
            pass

        try:
            self.enums.append(Enum(line, source))
            continue
        except Definition.NoMatch:
            pass

        try:
            self.constants.append(Constant(line, source))
            continue
        except Definition.NoMatch:
            pass

        try:
            self.branches.append(RefBranch(line))
            continue
        except Definition.NoMatch:
            pass

        try:
            self.branches.append(RefVectorBranch(line))
            continue
        except Definition.NoMatch:
            pass

        try:
            self.objbranches.append(ObjBranch(line))
            continue
        except Definition.NoMatch:
            pass

        try:
            self.branches.append(Branch(line))
            continue
        except Definition.NoMatch:
            pass

        try:
            self.branches.append(GenericBranch(line))
            continue
        except Definition.NoMatch:
            pass

        try:
            self.references.append(Reference(line))
            continue
        except Definition.NoMatch:
            pass
        
        try:
            self.functions.append(Function(line, source))
            continue
        except Definition.NoMatch:
            pass

        break

Object.__init__ = __init__

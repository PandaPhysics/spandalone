import os
import re
from panda import *

template_path = os.path.dirname(os.path.dirname(__file__)) + '/templates'

class Generator(object):
    def __init__(self):
        self.cnames = []
        self.cfiles = {}
        self.requirements = {}

        self.outdir = '.'
        self.namespace = 'panda'
        self.preserve_custom = True

        self.std_vector_branches = False

    def objects_dir(self):
        if self.namespace == 'panda':
            return self.outdir + '/Objects' # historical
        else:
            return self.outdir + '/' + self.namespace

    def add_config(self, path):
        if not path.endswith('.def'):
            raise RuntimeError('Definition file must have file name ending with .def')
    
        cname = os.path.basename(path)[:-4]
    
        self.cnames.append(cname)
        self.cfiles[cname] = path
        self.requirements[cname] = []
    
        with open(path) as configFile:
            # <require> lines must be at the beginning of the file
            for line in configFile:
                if not line.startswith('<require '):
                    break
        
                self.requirements[cname].append(line.strip().replace('<require ', '').replace('>', ''))

    def initialize(self):
        def check_requirement(cname, chain = None):
            if chain is None:
                chain = [cname]
            else:
                chain.append(cname)

            for pre in self.requirements[cname]:
                if pre in chain:
                    raise RuntimeError('Circular requirement: ' + ' -> '.join(chain) + ' -> ' + pre)

                check_requirement(pre, chain)
                
            chain.pop()

        for cname in self.cnames:
            check_requirement(cname)

        # iteratively sort configs
        sorted_cnames = list(self.cnames)
        while True:
            swapped = False
            for cname, req in self.requirements.items():
                imax = 0
                for pre in req:
                    ip = sorted_cnames.index(pre)
                    ic = sorted_cnames.index(cname)
                    if ip > ic:
                        sorted_cnames[ip], sorted_cnames[ic] = sorted_cnames[ic], sorted_cnames[ip]
                        swapped = True
        
            if not swapped:
                break

        self.includes = [
            Include('#include "TTree.h"'),
            Include('#include "TString.h"'),
            Include('#include "Rtypes.h"')
        ]
        self.typedefs = []
        self.constants = []
        self.asserts = []
        self.enums = []
        self.functions = []
        self.phobjects = []
        self.trees = []
        self.branchname = None
        self.sizebranch = None

    def parse(self):
        # parse all config files
        for cname in self.cnames:
            configFile = open(self.cfiles[cname])
        
            while True:
                line = configFile.readline()
                if line == '':
                    break
        
                line = line.strip()
                
                if line == '':
                    continue

                if line.startswith('%'):
                    #comment line
                    continue

                if line.startswith('<require '):
                    continue

                if line.startswith('<namespace'):
                    matches = re.match('<namespace *(.+)>', line.strip())
                    self.namespace = matches.group(1)
                    continue

                if line.startswith('<std_vector_branches>'):
                    self.std_vector_branches = True
                    continue

                if line.startswith('<branch_name>'):
                    self.branchname = BranchName(configFile)
                    continue

                if line.startswith('<size_branch_name>'):
                    self.sizebranch = BranchName(configFile)
                    continue
       
                try:
                    self.includes.append(Include(line))
                    continue
                except Definition.NoMatch:
                    pass
        
                try:
                    self.typedefs.append(Typedef(line))
                    continue
                except Definition.NoMatch:
                    pass
        
                try:
                    self.constants.append(Constant(line, configFile))
                    continue
                except Definition.NoMatch:
                    pass
        
                try:
                    self.asserts.append(Assert(line))
                    continue
                except Definition.NoMatch:
                    pass
        
                try:
                    self.enums.append(Enum(line, configFile))
                    continue
                except Definition.NoMatch:
                    pass
        
                try:
                    self.functions.append(Function(line, configFile))
                    continue
                except Definition.NoMatch:
                    pass
        
                try:
                    self.phobjects.append(PhysicsObject(line, configFile))
                    continue
                except Definition.NoMatch:
                    pass
        
                try:
                    self.trees.append(Tree(line, configFile))
                    continue
                except Definition.NoMatch:
                    pass
        
                print 'Skipping unrecognized pattern:', line
        
            configFile.close()

        for objdef in self.phobjects:
            objdef.make_inheritance_chain()

        for treedef in self.trees:
            treedef.make_inheritance_chain()

    def write_cpp(self):
        """
        Write C++ code
        """

        self.write_constants_header()
        self.write_constants_src()

        # Physics objects
        for objdef in self.phobjects:
            self.write_phobj_header(objdef)
            self.write_phobj_src(objdef)

        # Tree entries
        for treedef in self.trees:
            self.write_tree_header(treedef)
            self.write_tree_src(treedef)

        self.write_linkdef()

        # Branch naming convention
        self.write_branch_name()

    def write_constants_header(self):
        # Global constants and functions (header)
        outname = '{dirname}/interface/Constants.h'.format(dirname = self.objects_dir())
        outfile = FileOutput(outname, preserve_custom = self.preserve_custom)

        includes = BufferOutput()
        for include in self.includes:
            include.write(includes)
       
        typedefs = BufferOutput()
        for typedef in self.typedefs:
            typedef.write(typedefs)
            typedefs.newline()

        enums = BufferOutput()
        for enum in self.enums:
            enum.write_decl(enums, context = 'global')
            enums.newline()

        constants = BufferOutput()
        for constant in self.constants:
            constant.write_decl(constants, context = 'global')
            constants.newline()

        asserts = BufferOutput()
        Assert.write(self.asserts, asserts)

        functions = BufferOutput()
        for function in self.functions:
            function.write_decl(functions, context = 'global')
            functions.newline()
        
        replacements = {
            'NAMESPACE': self.namespace,
            'INCLUDES': includes,
            'TYPEDEFS': typedefs,
            'ENUMS': enums,
            'CONSTANTS': constants,
            'ASSERTS': asserts,
            'FUNCTIONS': functions
        }

        outfile.write_from_template(template_path + '/interface/Constants.h', replacements)
        outfile.close()

    def write_constants_src(self):
        # Global constants and functions (source)
        outname = '{dirname}/src/Constants.cc'.format(dirname = self.objects_dir())
        outfile = FileOutput(outname, preserve_custom = self.preserve_custom)

        enums = BufferOutput()
        for enum in self.enums:
            enum.write_def(enums)

        functions = BufferOutput()
        for function in self.functions:
            function.write_def(functions)

        replacements = {
            'NAMESPACE': self.namespace,
            'ENUMS': enums,
            'FUNCTIONS': functions
        }
        
        outfile.write_from_template(template_path + '/src/Constants.cc', replacements)
        outfile.close()

    def write_phobj_header(self, objdef):
        # Header
        header_includes = BufferOutput()
        objdef.write_header_includes(header_includes)

        enums = BufferOutput()
        for enum in objdef.enums:
            enum.write_decl(enums, context = 'class')

        constants = BufferOutput()
        for constant in objdef.constants:
            constant.write_decl(constants, context = 'class')

        functions = BufferOutput()
        for function in objdef.functions:
            function.write_decl(functions, context = 'class')

        public_members = BufferOutput()
        objdef.write_inherited_public_members(public_members)
        objdef.write_public_members(public_members)

        protected_members = BufferOutput()
        objdef.write_inherited_protected_members(protected_members)
        objdef.write_protected_members(protected_members)

        if self.namespace != 'panda' and objdef.parent in ('Element', 'Singlet'):
            parent = 'panda::' + objdef.parent
        else:
            parent = objdef.parent

        replacements = {
            'NAMESPACE': self.namespace,
            'NAME': objdef.name,
            'INSTANTIABLE': objdef.instantiable,
            'PARENT': parent,
            'PHYS_PARENT': (objdef.parent not in ('Singlet', 'Element')),
            'INCLUDES': header_includes,
            'ENUMS': enums,
            'CONSTANTS': constants,
            'FUNCTIONS': functions,
            'PUBLIC_MEMBERS': public_members,
            'PROTECTED_MEMBERS': protected_members,
        }

        if not objdef.is_singlet:
            buf = BufferOutput()
            if self.std_vector_branches:
                prot = BufferOutput()
                objdef.write_datastore_vectorptrs(prot)

                if len(prot.buffer) != 0:
                    buf.indent -= 1
                    buf.writeline('protected:')
                    buf.indent += 1
                    for _, line in prot.buffer:
                        buf.writeline(line)
                    buf.newline()
                    buf.indent -= 1
                    buf.writeline('public:')
                    buf.indent += 1

            objdef.write_datastore_inherited_members(buf, self.std_vector_branches)
            objdef.write_datastore_members(buf, self.std_vector_branches)

            replacements['DATASTORE_MEMBERS'] = buf

        outname = '{dirname}/interface/{obj}.h'.format(dirname = self.objects_dir(), obj = objdef.name)
        header = FileOutput(outname, preserve_custom = self.preserve_custom)
        if objdef.is_singlet:
            header.write_from_template(template_path + '/interface/singlet.h', replacements)
        else:
            header.write_from_template(template_path + '/interface/element.h', replacements)

    def write_phobj_src(self, objdef):
        # Source
        enum_strs = BufferOutput()
        for enum in objdef.enums:
            enum.write_def(enum_strs, cls = objdef.name)

        constants = BufferOutput()
        for constant in objdef.constants:
            constant.write_def(constants, cls = objdef.name)

        functions = BufferOutput()
        for function in objdef.functions:
            function.write_def(functions, cls = objdef.name)

        initializers_default = BufferOutput()
        objdef.write_initializers_default(initializers_default)

        initializers_copy = BufferOutput()
        objdef.write_initializers_copy(initializers_copy)

        bnames_list = []
        for cls in [objdef] + objdef.inheritance:
            bnames_list.extend(reversed([b.name for b in cls.branches if '!' not in b.modifier]))
        bnames_list.reverse()

        bnames = ', '.join('"{name}"'.format(name = b) for b in bnames_list)
        
        default_ctor = BufferOutput()
        copy_ctor = BufferOutput()
        assignment = BufferOutput()
        set_status = BufferOutput()
        set_address = BufferOutput()
        book = BufferOutput()
        init = BufferOutput()
        dump = BufferOutput()

        if objdef.is_singlet:
            context = 'Singlet'
            dscontext = 'Singlet'
        else:
            bnames_with_size = ', '.join('"{name}"'.format(name = b) for b in (['size'] + bnames_list))

            initializers_standard = BufferOutput()
            objdef.write_initializers_element(initializers_standard)
            initializers_private = BufferOutput()
            objdef.write_initializers_private(initializers_private)

            standard_ctor = BufferOutput()
            allocate = BufferOutput()
            deallocate = BufferOutput()
            release_tree = BufferOutput()
            resize_vectors = BufferOutput()

            context = 'Element'
            dscontext = 'datastore'

        for branch in objdef.branches:
            branch.write_default_ctor(default_ctor, context = context)
            branch.write_copy_ctor(copy_ctor, context = context)
            branch.write_assign(assignment, context = context)
            branch.write_set_status(set_status, context = dscontext)
            branch.write_set_address(set_address, context = dscontext, use_std_vector = self.std_vector_branches)
            branch.write_book(book, context = dscontext, use_std_vector = self.std_vector_branches)
            branch.write_init(init, context = context)
            branch.write_dump(dump)

            if not objdef.is_singlet:
                branch.write_standard_ctor(standard_ctor, context = context)
                branch.write_allocate(allocate, context = dscontext, use_std_vector = self.std_vector_branches)
                branch.write_deallocate(deallocate, context = dscontext, use_std_vector = self.std_vector_branches)
                branch.write_release_tree(release_tree, context = dscontext)
                branch.write_resize_vectors(resize_vectors, context = dscontext)

        if self.namespace != 'panda' and objdef.parent in ('Element', 'Singlet'):
            parent = 'panda::' + objdef.parent
        else:
            parent = objdef.parent

        replacements = {
            'NAMESPACE': self.namespace,
            'NAME': objdef.name,
            'PARENT': parent,
            'PHYS_PARENT': (objdef.parent not in ('Singlet', 'Element')),
            'STD_VECTOR_BRANCHES': self.std_vector_branches,
            'INSTANTIABLE': objdef.instantiable,
            'INITIALIZERS_DEFAULT': initializers_default,
            'INITIALIZERS_COPY': initializers_copy,
            'DEFAULT_CTOR': default_ctor,
            'COPY_CTOR': copy_ctor,
            'ENUM_STRS': enum_strs,
            'CONSTANTS': constants,
            'BNAMES': bnames,
            'ASSIGNMENT': assignment,
            'SET_STATUS': set_status,
            'SET_ADDRESS': set_address,
            'BOOK': book,
            'INIT': init,
            'DUMP': dump,
            'FUNCTIONS': functions
        }

        if not objdef.is_singlet:
            destructor = BufferOutput()
            objdef.write_destructor(destructor)

            replacements.update({
                'BNAMES_WITH_SIZE': bnames_with_size,
                'INITIALIZERS_STANDARD': initializers_standard,
                'INITIALIZERS_PRIVATE': initializers_private,
                'STANDARD_CTOR': standard_ctor,
                'ALLOCATE': allocate,
                'DEALLOCATE': deallocate,
                'RELEASE_TREE': release_tree,
                'RESIZE_VECTORS': resize_vectors,
                'DESTRUCTOR': destructor
            })

        outname = '{dirname}/src/{obj}.cc'.format(dirname = self.objects_dir(), obj = objdef.name)
        source = FileOutput(outname, preserve_custom = self.preserve_custom)
        if objdef.is_singlet:
            source.write_from_template(template_path + '/src/singlet.cc', replacements)
        else:
            source.write_from_template(template_path + '/src/element.cc', replacements)

    def write_branch_name(self):
        # Size branch name for collections
        replacements = {'NAMESPACE': self.namespace}

        if self.branchname is None:
            replacements['CUSTOM_BRANCHNAME'] = False
        else:
            parse = BufferOutput()
            self.branchname.write_parse(parse)
            generate = BufferOutput()
            self.branchname.write_generate(generate)

            replacements.update({
                'CUSTOM_BRANCHNAME': True,
                'BRANCHNAME_PARSE': parse,
                'BRANCHNAME_GENERATE': generate
            })

        if self.sizebranch is None:
            replacements['CUSTOM_SIZEBRANCH'] = False
        else:
            parse = BufferOutput()
            self.sizebranch.write_parse(parse)
            generate = BufferOutput()
            self.sizebranch.write_generate(generate)

            replacements.update({
                'CUSTOM_SIZEBRANCH': True,
                'SIZEBRANCH_PARSE': parse,
                'SIZEBRANCH_GENERATE': generate
            })

        outname = '{dirname}/interface/BranchName.h'.format(dirname = self.objects_dir())
        outfile = FileOutput(outname)
        outfile.write_from_template(template_path + '/interface/BranchName.h', replacements)
        outfile.close()
        outname = '{dirname}/src/BranchName.cc'.format(dirname = self.objects_dir())
        outfile = FileOutput(outname)
        outfile.write_from_template(template_path + '/src/BranchName.cc', replacements)
        outfile.close()            

    def write_tree_header(self, treedef):
        header_includes = BufferOutput()
        treedef.write_header_includes(header_includes)

        functions = BufferOutput()
        for function in treedef.functions:
            function.write_decl(functions, context = 'class')

        objbranches = BufferOutput()
        for objbranch in treedef.objbranches:
            objbranch.write_decl(objbranches)

        branches = BufferOutput()
        for branch in treedef.branches:
            branch.write_decl(branches, context = 'TreeEntry')

        if self.namespace != 'panda' and treedef.parent == 'TreeEntry':
            parent = 'panda::TreeEntry'
        else:
            parent = treedef.parent

        replacements = {
            'NAMESPACE': self.namespace,
            'NAME': treedef.name,
            'PARENT': parent,
            'INCLUDES': header_includes,
            'FUNCTIONS': functions,
            'OBJBRANCHES': objbranches,
            'BRANCHES': branches
        }

        outname = '{dirname}/interface/{tree}.h'.format(dirname = self.objects_dir(), tree = treedef.name)
        header = FileOutput(outname, preserve_custom = self.preserve_custom)
        header.write_from_template(template_path + '/interface/tree.h', replacements)

    def write_tree_src(self, treedef):
        collections = [b for b in treedef.objbranches if b.conttype == 'Collection']

        init_objptrs = BufferOutput()
        if len(treedef.objbranches) != 0:
            ptr_list = ', '.join(['&{name}'.format(name = b.name) for b in treedef.objbranches])
            init_objptrs.writeline('std::vector<panda::Object*> myObjects{std::initializer_list<panda::Object*>{' + ptr_list + '}};')
            init_objptrs.writeline('objects_.insert(objects_.end(), myObjects.begin(), myObjects.end());')

        init_collptrs = BufferOutput()
        if len(collections) != 0:
            ptr_list = ', '.join(['&{name}'.format(name = b.name) for b in collections])
            init_collptrs.writeline('std::vector<panda::CollectionBase*> myCollections{std::initializer_list<panda::CollectionBase*>{' + ptr_list + '}};')
            init_collptrs.writeline('collections_.insert(collections_.end(), myCollections.begin(), myCollections.end());')

        init_refs = BufferOutput()
        for ref in treedef.references:
            ref.write_def(init_refs, treedef.objbranches)

        initializers_copy = BufferOutput()
        treedef.write_initializers_copy(initializers_copy)

        assignments = BufferOutput()
        for branch in treedef.branches:
            branch.write_assign(assignments, context = 'TreeEntry')
        for objbranch in treedef.objbranches:
            objbranch.write_assign(assignments)
        for ref in treedef.references:
            ref.write_def(assignments, treedef.objbranches)

        dump = BufferOutput()
        for branch in treedef.branches:
            branch.write_dump(dump)
        dump.writeline('_out << std::endl;')
        for objbranch in treedef.objbranches:
            dump.writeline('_out << indentation << "{{" << {name}.getName() << "}}: " << {name}.typeName() << std::endl;'.format(name = objbranch.name))
            objbranch.write_dump(dump)

        bnames_list = []
        for cls in [treedef] + treedef.inheritance:
            bnames_list.extend(reversed([b.name for b in cls.branches if '!' not in b.modifier]))
        bnames_list.reverse()

        bnames = ', '.join('"{name}"'.format(name = b) for b in bnames_list)

        functions = BufferOutput()
        for function in treedef.functions:
            function.write_def(src, context = treedef.name)

        set_status = BufferOutput()
        set_address = BufferOutput()
        book = BufferOutput()
        init = BufferOutput()

        for branch in treedef.branches:
            branch.write_set_status(set_status, context = 'TreeEntry')
            branch.write_set_address(set_address, context = 'TreeEntry')
            branch.write_book(book, context = 'TreeEntry')
            branch.write_init(init, context = 'TreeEntry')

        if self.namespace != 'panda' and treedef.parent == 'TreeEntry':
            parent = 'panda::TreeEntry'
        else:
            parent = treedef.parent

        replacements = {
            'NAMESPACE': self.namespace,
            'NAME': treedef.name,
            'PARENT': parent,
            'PHYS_PARENT': (treedef.parent != 'TreeEntry'),
            'INIT_OBJPTRS': init_objptrs,
            'INIT_COLLPTRS': init_collptrs,
            'INIT_REFS': init_refs,
            'INITIALIZERS_COPY': initializers_copy,
            'ASSIGNMENTS': assignments,
            'DUMP': dump,
            'BNAMES': bnames,
            'FUNCTIONS': functions,
            'SET_STATUS': set_status,
            'SET_ADDRESS': set_address,
            'BOOK': book,
            'INIT': init
        }

        outname = '{dirname}/src/{tree}.cc'.format(dirname = self.objects_dir(), tree = treedef.name)
        source = FileOutput(outname, preserve_custom = self.preserve_custom)
        source.write_from_template(template_path + '/src/tree.cc', replacements)

    def write_linkdef(self):
        # write a linkdef file
        outname = '{dirname}/src/LinkDef.h'.format(dirname = self.objects_dir())
        outfile = FileOutput(outname)

        includes = BufferOutput()
        for objdef in self.phobjects:
            includes.writeline('#include "{dirname}/interface/{name}.h"'.format(dirname = os.path.realpath(self.objects_dir()), name = objdef.name))
        for tree in self.trees:
            includes.writeline('#include "{dirname}/interface/{name}.h"'.format(dirname = os.path.realpath(self.objects_dir()), name = tree.name))
        
        enums = BufferOutput()
        for enum in self.enums:
            enums.writeline('#pragma link C++ enum @NAMESPACE@::{name};'.format(name = enum.name))

        objects = BufferOutput()
        generics = BufferOutput()
        containers = BufferOutput()
        for objdef in self.phobjects:
            objects.writeline('#pragma link C++ class @NAMESPACE@::{name};'.format(name = objdef.name))

            for branch in objdef.branches:
                if type(branch) is GenericBranch:
                    generics.writeline('#pragma link C++ class {type}+;'.format(type = branch.type))
                    generics.writeline('#pragma link C++ class std::vector<{type}>+;'.format(type = branch.type))

            if not objdef.is_singlet:
                containers.writeline('#pragma link C++ class panda::Array<@NAMESPACE@::{name}>;'.format(name = objdef.name))
                containers.writeline('#pragma link C++ class panda::Collection<@NAMESPACE@::{name}>;'.format(name = objdef.name))
                containers.writeline('#pragma link C++ class panda::Ref<@NAMESPACE@::{name}>;'.format(name = objdef.name))
                containers.writeline('#pragma link C++ class panda::RefVector<@NAMESPACE@::{name}>;'.format(name = objdef.name))
                containers.writeline('#pragma link C++ typedef @NAMESPACE@::{name}Array;'.format(name = objdef.name))
                containers.writeline('#pragma link C++ typedef @NAMESPACE@::{name}Collection;'.format(name = objdef.name))
                containers.writeline('#pragma link C++ typedef @NAMESPACE@::{name}Ref;'.format(name = objdef.name))
                containers.writeline('#pragma link C++ typedef @NAMESPACE@::{name}RefVector;'.format(name = objdef.name))

        trees = BufferOutput()
        for tree in self.trees:
            trees.writeline('#pragma link C++ class @NAMESPACE@::{name};'.format(name = tree.name))

        replacements = {
            'OUTDIR': os.path.realpath(self.outdir),
            'NAMESPACE': self.namespace,
            'CUSTOM_NAMESPACE': (self.namespace != 'panda'),
            'INCLUDES': includes,
            'ENUMS': enums,
            'OBJECTS': objects,
            'GENERICS': generics,
            'CONTAINERS': containers,
            'TREES': trees
        }
        
        outfile.write_from_template(template_path + '/src/LinkDef.h', replacements)

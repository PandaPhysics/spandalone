from panda.base import Definition

class Branch(Definition):
    """
    Single branch definition. Definition file syntax:
    <name>([size])/<type>( = <init>)
    where <type> can be an object name or ROOT leaf type symbol.
    """

    TYPE_MAP = {'C': 'Text_t const*', 'B': 'Char_t', 'b': 'UChar_t', 'S': 'Short_t', 's': 'UShort_t',
        'I': 'Int_t', 'i': 'UInt_t', 'L': 'Long64_t', 'l': 'ULong64_t', 'F': 'Float_t', 'D': 'Double_t', 'O': 'Bool_t'}

    def __init__(self, line):
        Definition.__init__(self, line, '([a-zA-Z_][a-zA-Z0-9_]*)(|\\[.+\\])/([^ /]+)(?:|/([!m]+))(?:| += +([a-zA-Z0-9_.-]+))(?:| *( //.+))$')

        self.type = self.matches.group(3)
        if self.type not in Branch.TYPE_MAP:
            raise Definition.NoMatch()

        self.name = self.matches.group(1)
        # is this an array branch?
        arrdef = self.matches.group(2)
        if arrdef:
            self.arrdef = arrdef.strip('[]').split('][')
        else:
            self.arrdef = []

        self.modifier = self.matches.group(4)
        if self.modifier is None:
            self.modifier = ''

        self.init = self.matches.group(5) # used in decl
        if self.init is None:
            self.init = ''

        # initializer: used in init()
        if self.init:
            init = self.init
        elif self.type == 'O':
            init = 'false'
        elif self.type in 'FD':
            init = '0.'
        else:
            init = '0'

        if self.is_array():
            self.initializer = ''
            arr = self.name
            for iarr in range(len(self.arrdef)):
                self.initializer += 'for (auto& p{iarr} : {arr}) '.format(iarr = iarr, arr = arr)
                arr = 'p{iarr}'.format(iarr = iarr)
            self.initializer += 'p{iarr} = {init};'.format(iarr = iarr, init = init)
        else:
            self.initializer = '{name} = {init};'.format(name = self.name, init = init)

        self.comment = self.matches.group(6)
        if self.comment is None:
            self.comment = ''

    def is_array(self):
        # True if the branch itself is an array
        return len(self.arrdef) != 0

    def vartype(self):
        res = self.typename()
        for size in reversed(self.arrdef):
            res = 'std::array<%s, %d>' % (res, size)

        return res

    def arrdef_text(self, begin = None, end = None):
        return ''.join('[%s]' % a for a in self.arrdef[begin:end])

    def typename(self):
        return Branch.TYPE_MAP[self.type]

    def write_decl(self, out, context, use_std_vector = False):
        if context == 'datastore':
            if use_std_vector:
                template = 'std::vector<{vartype}>* {name}{{0}};'
            else:                
                template = '{vartype}* {name}{{0}};'
        elif context == 'Singlet' or context == 'TreeEntry':
            if 'm' in self.modifier:
                template = 'mutable {vartype} {name}{{{init}}};'
            else:
                template = '{vartype} {name}{{{init}}};'
        elif context == 'Element':
            template = '{vartype}& {name};'

        line = template.format(vartype = self.vartype(), name = self.name, init = self.init)

        if '!' in self.modifier:
            line += ' // transient'

        if context != 'datastore':
            line += self.comment

        out.writeline(line)

    def write_allocate(self, out, context, use_std_vector = False):
        # context must be datastore
        if use_std_vector:
            out.writeline('{name} = new std::vector<{vartype}>(nmax_);'.format(name = self.name, vartype = self.vartype()))
        else:
            out.writeline('{name} = new {vartype}[nmax_];'.format(name = self.name, vartype = self.vartype()))

    def write_deallocate(self, out, context, use_std_vector = False):
        # context must be datastore
        if use_std_vector:
            out.writeline('delete {name};'.format(name = self.name))
        else:
            out.writeline('delete [] {name};'.format(name = self.name))

        out.writeline('{name} = 0;'.format(name = self.name))

    def write_set_status(self, out, context):
        if '!' in self.modifier:
            return

        if context == 'datastore':
            namevar = '_name'
        elif context == 'Singlet':
            namevar = 'name_'
        elif context == 'TreeEntry':
            namevar = '""'

        out.writeline('panda::utils::setStatus(_tree, {namevar}, "{name}", _branches);'.format(namevar = namevar, name = self.name))

    def write_get_status(self, out, context):
        if '!' in self.modifier:
            return

        if context == 'datastore':
            namevar = '_name'
        elif context == 'Singlet':
            namevar = 'name_'
        elif context == 'TreeEntry':
            namevar = '""'

        out.writeline('blist.push_back(panda::utils::getStatus(_tree, {namevar}, "{name}"));'.format(namevar = namevar, name = self.name))

    def write_set_address(self, out, context, use_std_vector = False):
        if '!' in self.modifier:
            return

        if context == 'datastore':
            namevar = '_name'
        elif context == 'Singlet':
            namevar = 'name_'
        elif context == 'Element':
            namevar = '_name'
        elif context == 'TreeEntry':
            namevar = '""'

        if (context == 'datastore' and not use_std_vector) or self.is_array():
            ptr = self.name
        else:
            ptr = '&' + self.name

        out.writeline('panda::utils::setAddress(_tree, {namevar}, "{name}", {ptr}, _branches, _setStatus);'.format(namevar = namevar, name = self.name, ptr = ptr))

    def write_book(self, out, context, use_std_vector = False):
        if '!' in self.modifier:
            return

        if context == 'datastore' and use_std_vector:
            out.writeline('panda::utils::book(_tree, _name, "{name}", "std::vector<{vartype}>", &{name}, _branches);'.format(vartype = self.vartype(), name = self.name))
            return

        if self.is_array():
            # form an arrdef_text, where numeric literals are in quotes but the constants are in the code
            size_str = 'TString::Format("'
            args = []
            for a in self.arrdef:
                try:
                    size_str += '[' + str(int(a)) + ']'
                except ValueError:
                    size_str += '[%d]'
                    args.append(a)
    
            size_str += '"'
            if len(args):
                size_str += ', ' + ', '.join(args)
            size_str += ')'
        else:
            size_str = '"{arrdef}"'.format(arrdef = self.arrdef_text())

        if context == 'datastore':
            namevar = '_name'
            if self.is_array():
                size_str = 'size + ' + size_str
            else:
                size_str = 'size'
        elif context == 'Singlet':
            namevar = 'name_'
        elif context == 'Element':
            namevar = '_name'
        elif context == 'TreeEntry':
            namevar = '""'

        if context == 'datastore' or self.is_array():
            ptr = self.name
        else:
            ptr = '&' + self.name

        out.writeline('panda::utils::book(_tree, {namevar}, "{name}", {size}, \'{type}\', {ptr}, _branches);'.format(namevar = namevar, name = self.name, size = size_str, type = self.type, ptr = ptr))

    def write_release_tree(self, out, context):
        if '!' in self.modifier:
            return

        if context == 'datastore':
            namevar = '_name'
        elif context == 'Singlet':
            namevar = 'name_'
        elif context == 'Element':
            namevar = '_name'
        elif context == 'TreeEntry':
            namevar = '""'

        out.writeline('panda::utils::resetAddress(_tree, {namevar}, "{name}");'.format(namevar = namevar, name = self.name))

    def write_resize_vectors(self, out, context):
        pass

    def init_default(self, lines, context):
        if context == 'Element':
            lines.append('{name}(gStore.getData(this).{name}[0])'.format(name = self.name))

    def init_standard(self, lines, context):
        if context == 'Element':
            lines.append('{name}(_data.{name}[_idx])'.format(name = self.name))

    def init_copy(self, lines, context):
        if context == 'Singlet' or context == 'TreeEntry':
            if not self.is_array():
                lines.append('{name}(_src.{name})'.format(name = self.name))
        elif context == 'Element':
            lines.append('{name}(gStore.getData(this).{name}[0])'.format(name = self.name))

    def write_default_ctor(self, out, context):
        pass

    def write_standard_ctor(self, out, context):
        pass

    def write_copy_ctor(self, out, context):
        self.write_assign(out, context)

    def write_assign(self, out, context):
        if self.is_array():
            size = ' * '.join(self.arrdef)
            out.writeline('std::memcpy({name}, _src.{name}, sizeof({type}) * {size});'.format(name = self.name, type = self.typename(), size = size))
        else:
            out.writeline('{name} = _src.{name};'.format(name = self.name))

    def write_init(self, out, context):
        out.writeline(self.initializer)

    def write_dump(self, out):
        translations = {   # Solve the generic problem of casting, just in case
            'B': 'I',      # Cast chars to ints
            'b': 'i'       # Unsigned
        }

        translation = Branch.TYPE_MAP.get(translations.get(self.type)) if not self.is_array() else None
        cast = 'static_cast<const {outtype}>({name})'.format(outtype = translation, name = self.name) if translation else self.name

        out.writeline('_out << "{name} = " << {cast} << std::endl;'.format(name = self.name, cast = cast))

import os
import random
import string

import inspect


class defer:
    """
    Proof of concept for a python equivalent of golang's defer statement

    Note that the callback order is probably not guaranteed

    """
    def __init__(self, callback, *args, **kwargs):
        self.callback = callback
        self.args = args
        self.kwargs = kwargs

        # Add a reference to self in the caller variables so our __del__
        # method will be called when the function goes out of scope
        caller = inspect.currentframe().f_back
        caller.f_locals[b'_' + os.urandom(48)] = self

    def __del__(self):
        self.callback(*self.args, **self.kwargs)

builtin_types = ["bool", "char", "unsigned char", "short", "unsigned short", "float", "int", "unsigned int", "long",
                 "unsigned long", "double", "long double", "long long", "unsigned long long"]

restrictions = {}

output_dir = "generated"


def open_file(fn, mode):
    path = os.path.dirname(os.path.join(output_dir, fn))
    if not os.path.exists(path):
        os.makedirs(path)
    return open(os.path.join(output_dir, fn), mode)

# returns the 8 character long filename for the given message name, to be used as the header name
# limited to 8.3 under DOS
eight_names = {}
def eight_len_fn(name):
    if name in eight_names:
        return eight_names[name]

    eight_name = name.lower()[0:8]
    used = False
    # See if it is already used or not
    for enk in eight_names:
        env = eight_names[enk]
        if env == eight_name:
            used = True
            break
    if not used:
        eight_names[name] = eight_name.lower()
        return eight_name.lower()
    else:
        name_ctr = 1
        flen = 6
        while used:
            eight_name = name.lower()[0:flen] + str(name_ctr)
            used = False
            for enk in eight_names:
                env = eight_names[enk]
                if env == eight_name:
                    used = True
                    break
            if not used:
                eight_names[name] = eight_name.lower()
                return eight_name.lower()
            else:
                name_ctr += 1
                flen -= 1
                if flen == 0:
                    print("Too many identifiers used:", eight_names)
                    exit(2)



# True for the types which are serialized as a single text node (not lists)
def is_scalar(attr):
    return attr.strip().startswith("string") or attr in builtin_types or attr == "sequence"


# For "list of X" returns X
def list_item_type(attr):
    spltd = attr.split()
    if len(spltd) != 3 or spltd[0] != "list" or spltd[1] != "of":
        print("Invalid attribute: ", attr, ". Expected 'list of <type>'")
        exit(2)
    return spltd[2]


# The C++ expression which turns the given scalar member into XML text
def to_xml_text(attr, member):
    if attr.strip().startswith("string"):
        return "xml_escape(" + member + ")"
    if attr == "bool":
        return 'std::string(' + member + ' ? "1" : "0")'
    return "stringify(" + member + ")"


# The C++ statement which reads the given scalar member from XML text
def from_xml_text(attr, member, txt):
    if attr.strip().startswith("string"):
        return member + " = " + txt
    if attr == "bool":
        return member + " = xml_to_bool(" + txt + ")"
    if attr in ["int", "short", "sequence"]:
        return member + " = atoi(" + txt + ")"
    if attr in ["long", "long long"]:
        return member + " = atol(" + txt + ")"
    if attr.startswith("unsigned"):
        return member + " = strtoul(" + txt + ", NULL, 10)"
    if attr in ["float", "double", "long double"]:
        return member + " = atof(" + txt + ")"
    if attr == "char":
        return member + " = " + txt + "[0]"
    print("Cannot deserialize type:", attr)
    exit(2)


# True if the class is a message, ie. Message or something derived from it
def is_message(cls, pclasses):
    if cls["name"] == "Message":
        return True
    for e in cls["extends"]:
        if is_message(cfn(e, pclasses), pclasses):
            return True
    return False

# Used for the member declarations
def contained_attribute_type(attr, cls):
    if attr.strip().startswith("string"):
        return "std::string"
    if attr in builtin_types:
        return attr
    if attr.startswith("list"):
        spltd = attr.split()
        if len(spltd) < 3 or spltd[1] != "of":
            print("[3] Invalid attribute: ", attr, ". Missing 'of' keyword")
            exit(2)
        return spltd[2]
    if attr == "sequence":
        return "int"
    if attr == cls["name"]:
        return "const " + attr + "*"
    # Last resort
    return attr
    print("[1] Invalid attribute:", attr, " for ", cls["name"])
    exit(2)


# Used for the member declarations
def valid_attribute_type(attr, cls):
    if attr.strip().startswith("string"):
        return "std::string"
    if attr in builtin_types:
        return attr
    if attr.startswith("list"):
        spltd = attr.split()
        if spltd[1] != "of":
            print("Invalid attribute: ", attr, ". Missing 'of' keyword")
            exit(2)
        return "std::vector<" + valid_attribute_type(spltd[2], cls) + ">"
    if attr == "sequence":
        return "int"
    if attr == cls["name"]:
        return "const " + attr + "*"
    # Last resort
    return attr
    print("[1] Invalid attribute:", attr, " for ", cls["name"])
    exit(2)


# Used in the function calls, to pass in a const reference if possible
def const_if_can_attribute_type(attr, cls):
    if attr.startswith("string"):
        return "const std::string&"
    if attr in builtin_types:
        return attr
    if attr.startswith("list"):
        spltd = attr.split()
        if spltd[1] != "of":
            print("Invalid attribute: ", attr, ". Missing 'of' keyword")
            exit(2)
        return "const std::vector<" + valid_attribute_type(spltd[2], cls) + ">&"
    if attr == "sequence":
        return "int"
    if attr == cls["name"]:
        return "const " + attr + "*"
    print("[2] Invalid attribute: ", attr, " for ", cls["name"])
    exit(2)


# returns true if the parameter is a class defined in classes
def is_class_definition(name, classes):
    for cls in classes:
        if cls["name"] == name:
            return True
    return False


# gathers the includes for a header file that need to be included
def gather_atr_includes(cls, classes):
    includes = []
    if len(cls["attributes"]) > 0:
        for a in cls["attributes"]:
            attr = a["type"]
            if a in builtin_types:
                continue
            if is_class_definition(attr, classes):
                if attr not in includes:
                    includes.append(attr)
            if attr.startswith("list"):
                spltd = attr.split()
                if spltd[1] != "of":
                    print("Invalid attribute: ", attr, ". Missing 'of' keyword")
                    exit(2)
                if spltd[2] not in includes:
                    if spltd[2] in builtin_types or spltd[2] == "string":
                        continue
                    includes.append(spltd[2])
        return includes
    else:
        return []


# will gather all the attributes of a class, from the parent classes too
def all_attributes(cls, pclasses):
    basic_attributes = cls["attributes"]
    extended_attributes = []

    if "extends" in cls and len(cls["extends"]) > 0:
        for e in cls["extends"]:
            for c in pclasses:
                if c["name"] == e:
                    extended_attributes = extended_attributes + all_attributes(c, pclasses)

    return basic_attributes + extended_attributes


# Return the attributes of the specific class
def attributes_of(clsname, pclasses):
    for c in pclasses:
        if c["name"] == clsname:
            return c["attributes"]


# Returns the class dictionary for the given class name
def cfn(clsname, pclasses):
    for c in pclasses:
        if c["name"] == clsname:
            return c


# Generates the required header and cpp files
def generate(pclasses):
    global restrictions

    # first, the header
    for cls in pclasses:
        class_name = cls["name"]
        f = open_file(eight_len_fn(cls["name"]) + ".h", "w")
        f.write("#ifndef __" + cls["name"].upper() + "_H__\n")
        f.write("#define __" + cls["name"].upper() + "_H__\n")

        # what needs to be included
        if len(cls["extends"]) > 0:
            for e in cls["extends"]:
                f.write("#include \"" + eight_len_fn(e)+ ".h\"\n")

        # the headers for the message types
        msg_includes = gather_atr_includes(cls, classes)
        for i in msg_includes:
            f.write("#include \"" + eight_len_fn(i) + ".h\"\n")

        # The headers for the builtin types (and string.h for the serializer)
        needs_to_be_included = ["<string>"]
        if len(cls["attributes"]) > 0:
            attr_types = [d.get("type", '') for d in cls["attributes"]]
            for a in attr_types:
                if a.startswith("list"):
                    if "<vector>" not in needs_to_be_included:
                        needs_to_be_included.append("<vector>")

        for tbi in needs_to_be_included:
            f.write("#include " + tbi + "\n")
        f.write("#include \"ezxml.h\"\n")

        # The class declaration
        f.write("class " + cls["name"])
        if len(cls["extends"]) > 0:
            f.write(": ")
            publics = []
            for e in cls["extends"]:
                publics.append("public " + e)
            f.write(",".join(publics))
        f.write("\n{\npublic:\n")

        restrictions[cls["name"]] = {}

        # Constructors. There are no default arguments on purpose: Open Watcom
        # silently drops the construction of objects whose constructor has
        # std::vector temporaries as default arguments.
        own_params = [a for a in cls["attributes"] if a["type"] != "sequence"]
        init = [e + "()" for e in cls["extends"]]
        for a in cls["attributes"]:
            init.append("m_" + a["name"] + ("(++ seq_" + a["name"] + ")" if a["type"] == "sequence" else "()"))
        f.write("    " + cls["name"] + "()" + (" : " + ", ".join(init) if init else "") + "\n    {}\n")
        if own_params:
            f.write("\n    " + cls["name"] + "(" + ", ".join(const_if_can_attribute_type(a["type"], cls) + " p_" + a["name"] for a in own_params) + ")")
            init = [e + "()" for e in cls["extends"]]
            for a in cls["attributes"]:
                init.append("m_" + a["name"] + ("(++ seq_" + a["name"] + ")" if a["type"] == "sequence" else "(p_" + a["name"] + ")"))
            f.write(" : " + ", ".join(init))
        # let's see if any of the values are restricted to something, these checks go into the body
        # of the constructor with parameters
        restricted = False
        opp_written = False
        for a in cls["attributes"]:
            attr = a["type"]
            if attr.find(" of ") != -1:
                restricted = True
                value_list = True
                if not opp_written:
                    f.write("\n    {")
                    opp_written = True
                spltd_type = a["type"].split('[')
                if len(spltd_type) != 2:
                    # See if this is the form "list of something"
                    list_split = a["type"].split(" ")
                    if len(list_split) == 3:
                        if list_split[0] != "list":
                            print("Invalid attribute: ", attr, ". Improper use of the 'of' keyword:", attr)
                            exit(2)
                        else:
                            value_list = False
                    else:
                        print("Invalid attribute: ", attr, ". Missing [ from the restriction list in:" ,attr)
                        exit(2)
                if value_list:
                    list_of_values = spltd_type[1].split(",")
                if value_list and not list_of_values[-1].endswith("]"):
                    print("Invalid attribute: ", attr, ". Missing ] from the restriction list")
                    exit(2)
                if value_list and len(list_of_values) > 0:
                    restrictions[cls["name"]][a["name"]] = []
                    f.write("\n        if(")
                    to_join = []
                    for rv in list_of_values:
                        rv = rv.replace("]", "")
                        if not rv.startswith('"') or not rv.endswith('"'):
                            print("Invalid attribute: ", attr, ". String not quoted properly")
                            exit(2)

                        restrictions[cls["name"]][a["name"]].append(rv)
                        to_join.append("m_" + a["name"] + " != " + rv)
                    f.write(" && ".join(to_join))
                    f.write(")\n        {\n            ")
                    if a["type"] in builtin_types:
                        f.write("m_" + a["name"] + " =  0")
                    else:
                        f.write("m_" + a["name"] + " = " + valid_attribute_type(a["type"], cls) + "();")
                    f.write("\n        }")

        if opp_written:
            f.write("\n    }\n")

        if not restricted and own_params:
            f.write("\n    {}\n")

        sequences = []

        # Destructor
        f.write("\n    virtual ~" + cls["name"] + "() {}\n")

        # The name of the message
        f.write("\n    virtual std::string name() const { return \"" + cls["name"] + "\";}\n")

        # Now the setters for the attributes
        f.write("\n    // setters\n")
        if len(cls["attributes"]) > 0:
            attr_names = [d.get("name", '') for d in cls["attributes"]]
            attr_types = [d.get("type", '') for d in cls["attributes"]]
            for a, t in zip(attr_names, attr_types):
                if t != "sequence":
                    f.write("    void set_" + a + "(" + const_if_can_attribute_type(t, cls) + " p_" + a + ");\n")
                else:
                    sequences.append(a)

        # Now the getters for the attributes
        f.write("\n    // getters\n")
        if len(cls["attributes"]) > 0:
            attr_names = [d.get("name", '') for d in cls["attributes"]]
            attr_types = [d.get("type", '') for d in cls["attributes"]]
            for a, t in zip(attr_names, attr_types):
                # strings and lists by reference, copying them is expensive on DOS
                rt = valid_attribute_type(t, cls)
                if rt.startswith("std::"):
                    rt = "const " + rt + "&"
                f.write("    " + rt + " get_" + a + "() const\n    {\n")
                f.write("        return  m_" + a + ";\n")
                f.write("    }\n")

        # serializer
        f.write("\n    // serializer\n")
        f.write("    virtual std::string serialize() const;\n")
        f.write("    virtual int deserialize(const char*);\n")
        f.write("    virtual int deserialize(ezxml_t);\n")

        # The equality operator
        f.write("\n    // comparison\n")
        f.write('    bool operator == (const ' + cls["name"] + "&) const;\n")

        # Now the attributes

        f.write("\nprotected:\n")
        f.write("    void serialize_attributes(std::string&) const;\n")
        f.write("    int deserialize_attributes(ezxml_t);\n")

        f.write("\nprivate:\n")
        if len(cls["attributes"]) > 0:
            attr_names = [d.get("name", '') for d in cls["attributes"]]
            attr_types = [d.get("type", '') for d in cls["attributes"]]
            for a, t in zip(attr_names, attr_types):
                f.write("    " + valid_attribute_type(t, cls) + " m_" + a + ";\n" )

        # All the sequences
        if len(sequences) > 0:
            f.write("\nprivate:\n")
            for seq in sequences:
                f.write("    static int seq_" + seq + ";\n")

        f.write("};\n")
        f.write("#endif\n")
        f.close()

        # Then the cpp
        f = open_file(eight_len_fn(cls["name"]) + ".cpp", "w")
        f.write("#include \"" + eight_len_fn(cls["name"]) + ".h\"\n")
        f.write("#include \"strngify.h\"\n\n")
        f.write("#include <string.h>\n")
        f.write("#include <stdlib.h>\n\n")

        if len(sequences) > 0:
            for seq in sequences:
                f.write("int " + cls["name"] + "::seq_" + seq + " = 0;\n")

        # The serializer function
        f.write('\nstd::string ' + cls["name"] + '::serialize() const\n{\n')
        f.write('    std::string result = "<o><type>' + cls["name"] + '</type>";\n')
        f.write('    result += "<attributes>";\n')
        f.write('    serialize_attributes(result);\n')
        f.write('    result += "</attributes></o>";\n')
        f.write('    return result;\n}\n')

        # The attributes, the ones of the base classes first
        f.write('\nvoid ' + cls["name"] + '::serialize_attributes(std::string& result) const\n{\n')
        for e in cls["extends"]:
            f.write('    ' + e + '::serialize_attributes(result);\n')
        for a in cls["attributes"]:
            f.write("    // attribute:" + a["name"] + "\n")
            f.write('    result += "<' + a["name"] + '>";\n')
            if is_scalar(a["type"]):
                f.write('    result += ' + to_xml_text(a["type"], "m_" + a["name"]) + ';\n')
            else:
                contained_type = list_item_type(a["type"])
                f.write('    for(size_t i=0; i<m_' + a["name"] + '.size(); i++)\n    {\n')
                if is_scalar(contained_type):
                    f.write('        result += "<item>" + ' + to_xml_text(contained_type, "m_" + a["name"] + "[i]") + ' + "</item>";\n')
                else:
                    f.write('        result += "<item>" + m_' + a["name"] + '[i].serialize() + "</item>";\n')
                f.write('    }\n')
            f.write('    result += "</' + a["name"] + '>";\n')
        f.write('}\n')

        # The deserializer from a string, parses it and hands over to the node one
        f.write('\nint ' + cls["name"] + '::deserialize(const char* xml)\n{\n')
        f.write('    size_t len = strlen(xml);\n')
        f.write('    char* copy = (char*)malloc(len + 1);\n')
        f.write('    if(!copy) return 0;\n')
        f.write('    memcpy(copy, xml, len + 1);\n')
        f.write('    ezxml_t x = ezxml_parse_str(copy, len);\n')
        f.write('    int result = deserialize(x);\n')
        f.write('    ezxml_free(x);\n')
        f.write('    free(copy);\n')
        f.write('    return result;\n}\n')

        # The deserializer from an already parsed <o> node
        f.write('\nint ' + cls["name"] + '::deserialize(ezxml_t x)\n{\n')
        f.write('    if(!x) return 0;\n')
        f.write('    ezxml_t type_node = ezxml_child(x, "type");\n')
        f.write('    if(!type_node || strcmp(type_node->txt, "' + cls["name"] + '")) return 0;\n')
        f.write('    return deserialize_attributes(ezxml_child(x, "attributes"));\n}\n')

        f.write('\nint ' + cls["name"] + '::deserialize_attributes(ezxml_t attrs_node)\n{\n')
        f.write('    if(!attrs_node) return 0;\n')
        for e in cls["extends"]:
            f.write('    if(!' + e + '::deserialize_attributes(attrs_node)) return 0;\n')
        for a in cls["attributes"]:
            node = 'attr_node_' + a["name"]
            f.write('    ezxml_t ' + node + ' = ezxml_child(attrs_node, "' + a["name"] + '");\n')
            if is_scalar(a["type"]):
                f.write('    if(' + node + ') ' + from_xml_text(a["type"], "m_" + a["name"], node + "->txt") + ';\n')
            else:
                contained_type = list_item_type(a["type"])
                f.write('    m_' + a["name"] + '.clear();\n')
                f.write('    for(ezxml_t item = ezxml_child(' + node + ', "item"); item; item = item->next)\n    {\n')
                f.write('        ' + valid_attribute_type(contained_type, cls) + ' l_item;\n')
                if is_scalar(contained_type):
                    f.write('        ' + from_xml_text(contained_type, "l_item", "item->txt") + ';\n')
                else:
                    f.write('        if(!l_item.deserialize(ezxml_child(item, "o"))) return 0;\n')
                f.write('        m_' + a["name"] + '.push_back(l_item);\n')
                f.write('    }\n')
        f.write('    return 1;\n}\n\n')

        # Comparison operator implemented
        f.write('bool ' + cls["name"] + '::operator == (const ' + cls["name"] + "& rhs) const\n{\n")
        for e in cls["extends"]:
            f.write("    if(!" + e + "::operator ==(rhs)) return false;\n")
        for a in cls["attributes"]:
            if is_scalar(a["type"]):
                f.write("    if(m_" + a["name"] + " != " + "rhs.m_" + a["name"] + ") return false;\n")
            else:
                f.write("    if(m_" + a["name"] + ".size() != " + "rhs.m_" + a["name"] + ".size() ) return false;\n")
                f.write("    for(size_t i_" + a["name"] + " = 0; i_" + a["name"] + " < m_" + a["name"] + ".size(); i_" + a["name"] + "++)\n")
                f.write("        if(!(m_" + a["name"] + "[i_" + a["name"] + "] == rhs.m_" + a["name"] + "[i_" + a["name"] + "])) return false;\n")

        f.write('\n    return true;\n}\n')

        # setters implemented
        if len(cls["attributes"]) > 0:
            attr_names = [d.get("name", '') for d in cls["attributes"]]
            attr_types = [d.get("type", '') for d in cls["attributes"]]
            for a, t in zip(attr_names, attr_types):
                if t != "sequence":
                    f.write("void " + cls["name"]+ "::set_" + a + "(" + const_if_can_attribute_type(t, cls) + " p_" + a + ")\n")
                    f.write("{")
                    if a in restrictions[cls["name"]]:
                        to_join = []
                        for r in restrictions[cls["name"]][a]:
                            to_join.append("p_" + a + " != " + r)
                        f.write("\n    if (" + " && ".join(to_join) + ") { return; }")
                    f.write("\n    m_" + a + " = p_" + a + ";")
                    f.write("\n}\n")
        f.close()


# will generate a constructor, default_v is whether there is a default value or not
def generate_constructor_init_list(cls, f, pclasses, default_v):
    attrs = []
    for a in all_attributes(cls, pclasses):
        cpar = const_if_can_attribute_type(a["type"], cls) + " p_" + a["name"]
        if default_v:
            if a["type"] in builtin_types or a["type"] == "sequence":
                cpar += " =  0"
            else:
                cpar += " = " + valid_attribute_type(a["type"], cls) + "()"

        attrs.append(cpar)
    f.write(", ".join(attrs))
    # Do we have an initializer list?
    if len(cls["extends"]) or len(cls["attributes"]) > 0:
        f.write(") : ")
    # Any base classes?
    if len(cls["extends"]) > 0:
        baseclass_calls = []
        for e in cls["extends"]:
            attrs_of_e = attributes_of(e, pclasses)
            t_values = [d.get("name", '') for d in attrs_of_e]
            values = ['p_' + element for element in t_values]
            result = ''.join(map(str, values))
            baseclass_calls.append(e + "(" + result + ")")
        f.write(", ".join(baseclass_calls))
    # do we need a comma between ?
    if len(cls["extends"]) > 0 and len(cls["attributes"]) > 0:
        f.write(", ")
    if len(cls["attributes"]) > 0:
        attr_names = ["m_" + d.get("name", '') for d in cls["attributes"]]
        attr_par_values = []
        for a in cls["attributes"]:
            if a["type"] != "sequence":
                attr_par_values.append("p_" + a["name"])
            else:
                attr_par_values.append("++ seq_" + a["name"])

        result_list = [str(val1) + "(" + str(val2) + ")" for val1, val2 in zip(attr_names, attr_par_values)]
        f.write(", ".join(result_list))


def remove_spaces_around_character(line, character):
    # Find the index of the specified character
    char_index = line.find(character)

    if char_index != -1:
        # Remove spaces before the specified character
        before_char = line[:char_index].rstrip()

        # Remove spaces after the specified character
        after_char = line[char_index + 1:].lstrip()

        # Combine the parts without spaces and the specified character
        result_line = before_char + character + after_char

        return result_line

    # If the character is not found, return the original line
    return line


#
# Builds the structures of the IDL
#
def build_structures(filename):
    global classes
    classes = []
    # open the file
    with open(filename) as file:
        lines = [line.rstrip() for line in file]

    # count the lines
    line_ctr = 0

    # check length
    if len(lines) == 0:
        print("Empty IDL file")
        exit(2)

    # 0 - load class names and derivations
    # 1 - the opening brace
    # 2 - load attributes
    # 3 - the closing brace
    state = 0

    # The current class we work on
    cls = {"attributes": [], "extends": []}

    # iterate the lines
    for line in lines:
        line_ctr = line_ctr + 1
        line = line.strip()

        if len(line) == 0:
            continue

        # Lines marked with // are comments
        if line[0] == '/':
            continue

        # opening an attribute list
        if line == "{":
            if state != 1:
                print("Unexpected { in line ", line_ctr)
                exit(2)
            state = 2
            continue

        # closing the attribute list, starting a new class definition
        if line == "}":
            if state != 2:
                print("Unexpected } in line ", line_ctr)
                exit(2)
            state = 0
            # reset the class
            cls = {"attributes": [], "extends": []}
            continue

        # remove unnecessary spaces
        line = ' '.join(line.split())
        while line.find(': ') != -1 or line.find(' :') != -1:
            line = remove_spaces_around_character(line, ':')

        if state == 2:
            attribute_and_type = line.split(":")
            if len(attribute_and_type) != 2:
                print("Invalid attribute definition in line: ", line, " line ", line_ctr)
                exit(2)

            ant = {"name": attribute_and_type[0], "type": attribute_and_type[1]}
            cls["attributes"].append(ant)
            continue

        if state == 0:
            if not line.startswith("class"):
                print("Unexpected class definition: ", line, " in line ", line_ctr)
                exit(2)
            class_and_name = line.split(" ")
            if len(class_and_name) < 2:
                print("Invalid class definition: ", class_and_name, " in line ", line_ctr)
                exit(2)
            class_name = class_and_name[1]
            # Does this extend a class?
            if class_name.find(":") != -1:
                classname_extends = class_name.split(":")
                cls["name"] = classname_extends[0]
                cls["extends"].append(classname_extends[1])
            else:
                cls["name"] = class_name
            classes.append(cls)
            state = 1

    return classes

#
# Generates a CMakeLists.txt for the project
#
def generate_make(pclasses):
    f = open_file("CMakeLists.txt", "w")
    f.write('project(test)\n')
    f.write('set(CMAKE_CXX_STANDARD 98)\n')
    f.write('include_directories(${CMAKE_SOURCE_DIR})\n')
    f.write('add_subdirectory(tests)\n')
    f.write("add_library(idl \nezxml.c\n")

    for cls in pclasses:
        f.write('    ' + eight_len_fn(cls["name"]) + ".cpp\n")
    f.write("    protocol.cpp\n")
    f.write("\n)")
    f.close()

    mf = open_file("makefile", "w")
    mf.write("protocol_objs = ")
    to_join = []
    for cls in pclasses:
        to_join.append(eight_len_fn(cls["name"]) + ".o")
    mf.write(" ".join(to_join) + " protocol.o\n")
    mf.close()

#
# Creates the "generated" folder and the subfolder required in it
#
def create_generated_folder(subf):
    path = output_dir
    does_exist = os.path.exists(path)
    if not does_exist:
        # Create a new directory because it does not exist
        os.makedirs(path)
    path = os.path.join(output_dir, subf)
    # Check whether the specified path exists or not
    does_exist = os.path.exists(path)
    if not does_exist:
        # Create a new directory because it does not exist
        os.makedirs(path)


#
# Will generate some test cases. Just in case.
#
def generate_tests(pclasses):
    create_generated_folder("tests")
    cm = open_file("tests/CMakeLists.txt", "w")
    cm.write('''
project("idl-test")    
Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain idl)   
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)
    ''')

    f = open_file("tests/tests.cpp", "w")

    # The includes
    f.write("# include <catch2/catch_test_macros.hpp>\n")
    f.write("# include <iostream>\n")

    f.write("#define private public\n")

    for cls in pclasses:
        f.write("#include <" + eight_len_fn(cls["name"]) + ".h>\n")

    # The random object generators
    f.write("\n")
    for cls in pclasses:
        f.write(cls["name"] + " random" + cls["name"] + '() {\n')
        f.write("    " + cls["name"] + " l" + cls["name"] + ";\n");
        for a in cls["attributes"]:
            attr = a["type"]

            f.write('    l' + cls["name"] + "." + a["name"] + " = ")
            if a["type"] in builtin_types:
                f.write(str(random.randint(1, 9)))
                f.write(";\n")
            else:
                if valid_attribute_type(a["type"], cls) == "std::string":
                    f.write('"' + ''.join(random.choices(string.ascii_uppercase + string.digits, k=8)) +'";\n' )
                else:
                    if attr.startswith("list"):
                        spltd = attr.split()
                        if spltd[1] != "of":
                            print("Invalid attribute: ", attr, ". Missing 'of' keyword")
                            exit(2)
                        randomObjects = []
                        for i in range(1, 4):
                            randomObjects.append("random" + spltd[2] + "()")
                        f.write("{" + ','.join(randomObjects) + "};\n")

        f.write("    return l" + cls["name"] + ";\n}\n")

    # The actual test cases
    f.write("\n")
    for cls in pclasses:
        f.write('\nTEST_CASE("' + cls["name"] + '", "[serialize/deserialize]") {\n')
        f.write("    " + cls["name"] + " randObj = random" + cls["name"] + "();\n")
        f.write("    std::string serialized = randObj.serialize();\n")
        f.write("std::cout << serialized << std::endl;")
        f.write("    " + cls["name"] + " deseredObj;\n")
        f.write('    if(deseredObj.deserialize(serialized.c_str()) == 0) FAIL_CHECK("expected timeout failure");\n')
        for a in cls["attributes"]:
            attrn = a["name"]
            f.write("    CHECK(randObj." + attrn + " == deseredObj." + attrn + ");\n")
        f.write("\n}\n")

#
# Breaks the string on whitespace
#
def break_string_on_whitespace(input_string):
    words = input_string.split()
    goesback = []
    result = []
    clen = 0
    for word in words:
        result.append(word)
        clen += len(word)
        if clen > 60 and word.startswith("p_"):
            goesback.append(" ".join(result))
            result = []
            result.append("        ")
            clen = 0

    return '\n'.join(goesback)

def search(attribute, attributes):
    return [element for element in attributes if element['name'] == attribute]

#
# Will generate the message receivers and deserializers
#
def generate_message_receiver(pclasses):

    messages = [cls for cls in pclasses if cls["name"] != "Message" and is_message(cls, pclasses)]

    # the parameters of the create_ functions: the own, non sequence attributes
    def creator_params(cls):
        return [a for a in cls["attributes"] if a["type"] != "sequence"]

    # protocol.h
    f = open_file("protocol.h", "w")
    f.write("#ifndef __PROTOCOL_H__\n")
    f.write("#define __PROTOCOL_H__\n\n")
    f.write("// Generated by tools/idl_gen/gen.py from main.idl, do not edit\n\n")
    f.write("#include \"ezxml.h\"\n")

    for cls in pclasses:
        f.write("#include <" + eight_len_fn(cls["name"]) + ".h>\n")

    f.write('\n// Protocol message handler types\n')
    for cls in messages:
        f.write("typedef void(*" + cls["name"] + "_Handler)")
        f.write("(const " + cls["name"] + "*);\n")

    f.write("\nclass Protocol {\npublic:\n")

    # Constructor
    f.write("\n    Protocol()")
    if len(messages) > 0:
        f.write(" : ")
        f.write(", ".join(["m_" + cls["name"] + "_handler(NULL)" for cls in messages]))
    f.write("\n    {}\n\n")
    f.write("    virtual ~Protocol() {}\n\n")

    f.write("    // message creators, the caller owns the returned object\n")
    for cls in messages:
        params = [const_if_can_attribute_type(a["type"], cls) + " p_" + a["name"] for a in creator_params(cls)]
        f.write("    " + cls["name"] + "* create_" + cls["name"] + "(" + ", ".join(params) + ");\n")

    f.write("\n    // message handler setters\n")
    for cls in messages:
        f.write("    void set_" + cls["name"] + "_Handler(" + cls["name"] + "_Handler p_handler) {\n")
        f.write("        m_" + cls["name"] + "_handler = p_handler;\n    }\n")

    f.write("\n    /**\n     * Deserializes the given <o> node as the message type and calls its handler.\n")
    f.write("     * Returns 1 if the message was handled, 0 otherwise.\n     **/\n")
    f.write("    int receive(const char* p_message_type, ezxml_t p_o);\n")

    f.write("\nprivate:\n\n")
    for cls in messages:
        f.write("    " + cls["name"] + "_Handler m_" + cls["name"] + "_handler;\n")

    f.write("\n};\n")
    f.write("#endif\n")
    f.close()

    # protocol.cpp
    f = open_file("protocol.cpp", "w")
    f.write("// Generated by tools/idl_gen/gen.py from main.idl, do not edit\n\n")
    f.write("#include \"protocol.h\"\n")
    f.write("#include <log.h>\n")
    f.write("#include <string.h>\n\n")

    f.write("int Protocol::receive(const char* p_message_type, ezxml_t p_o)\n{\n")
    for cls in messages:
        n = cls["name"]
        f.write('    if(!strcmp(p_message_type, "' + n + '"))\n    {\n')
        f.write('        if(!m_' + n + '_handler) { log_warning() << "No handler for ' + n + '"; return 0; }\n')
        f.write('        ' + n + ' obj;\n')
        f.write('        if(!obj.deserialize(p_o)) { log_error() << "Cannot deserialize ' + n + '"; return 0; }\n')
        f.write('        m_' + n + '_handler(&obj);\n')
        f.write('        return 1;\n    }\n')
    f.write('    log_warning() << "Unknown message:" << p_message_type;\n')
    f.write("    return 0;\n}\n\n")

    for cls in messages:
        params = creator_params(cls)
        decl = [const_if_can_attribute_type(a["type"], cls) + " p_" + a["name"] for a in params]
        f.write(cls["name"] + "* Protocol::create_" + cls["name"] + "(" + ", ".join(decl) + ")\n{\n")
        f.write("    return new " + cls["name"] + "(" + ", ".join(["p_" + a["name"] for a in params]) + ");\n}\n\n")
    f.close()


#
# Generates the python module used by the Linux peer
#
def generate_python(pclasses, out_file):
    def py_kind(attr):
        if attr.strip().startswith("string"):
            return "string"
        if attr == "bool":
            return "bool"
        if attr in ["float", "double", "long double"]:
            return "float"
        if is_scalar(attr):
            return "int"
        return "list"

    def py_default(attr):
        return {"string": '""', "bool": "False", "float": "0.0", "int": "0", "list": "None"}[py_kind(attr)]

    f = open(out_file, "w")
    f.write(PY_RUNTIME_HEAD)

    for cls in pclasses:
        base = cls["extends"][0] if cls["extends"] else "_Object"
        fields = []
        for a in cls["attributes"]:
            kind = py_kind(a["type"])
            item = kind
            if kind == "list":
                it = list_item_type(a["type"])
                item = it if is_class_definition(it, pclasses) else py_kind(it)
            fields.append('("' + a["name"] + '", "' + kind + '", "' + item + '"), ')
        f.write("\n\nclass " + cls["name"] + "(" + base + "):\n")
        f.write('    NAME = "' + cls["name"] + '"\n')
        f.write("    FIELDS = " + base + ".FIELDS + (" + "".join(fields) + ")\n")

        all_attrs = all_attributes(cls, pclasses)
        args = ["self"] + [a["name"] + "=" + py_default(a["type"]) for a in all_attrs if a["type"] != "sequence"]
        f.write("\n    def __init__(" + ", ".join(args) + "):\n")
        body = []
        for a in all_attrs:
            if a["type"] == "sequence":
                body.append("        self." + a["name"] + " = next(_sequence)")
            elif py_kind(a["type"]) == "list":
                body.append("        self." + a["name"] + " = " + a["name"] + " if " + a["name"] + " is not None else []")
            else:
                body.append("        self." + a["name"] + " = " + a["name"])
        f.write("\n".join(body) if body else "        pass")
        f.write("\n")

    f.write("\n\n_CLASSES = {\n")
    for cls in pclasses:
        f.write('    "' + cls["name"] + '": ' + cls["name"] + ",\n")
    f.write("}\n")
    f.write(PY_RUNTIME_TAIL)
    f.close()


PY_RUNTIME_HEAD = """# Generated by tools/idl_gen/gen.py from main.idl, do not edit
\"\"\"The cloudy message protocol, Python side.\"\"\"

import itertools
import xml.etree.ElementTree as ET
from xml.sax.saxutils import escape

_ENVELOPE_HEAD = '<?xml version="1.0" encoding="UTF-8" standalone="yes" ?><protocol><cld v="1.0" msg="'

_sequence = itertools.count(1)


class _Object:
    NAME = ""
    # (attribute name, kind, item class name or kind for lists)
    FIELDS = ()

    def serialize(self):
        out = ["<o><type>", self.NAME, "</type><attributes>"]
        for name, kind, item in self.FIELDS:
            value = getattr(self, name)
            out.append("<" + name + ">")
            if kind == "list":
                for v in value:
                    out.append("<item>" + _to_text(item, v) + "</item>")
            else:
                out.append(_to_text(kind, value))
            out.append("</" + name + ">")
        out.append("</attributes></o>")
        return "".join(out)

    @classmethod
    def from_node(cls, o):
        if o is None or o.findtext("type") != cls.NAME:
            return None
        attrs = o.find("attributes")
        if attrs is None:
            return None
        obj = cls()
        for name, kind, item in cls.FIELDS:
            node = attrs.find(name)
            if node is None:
                continue
            if kind == "list":
                setattr(obj, name, [_from_item(item, i) for i in node.findall("item")])
            else:
                setattr(obj, name, _from_text(kind, node.text))
        return obj

    def __repr__(self):
        return self.NAME + "(" + ", ".join(n + "=" + repr(getattr(self, n)) for n, _, _ in self.FIELDS) + ")"


def _to_text(kind, value):
    if kind in _CLASSES:
        return value.serialize()
    if kind == "string":
        return escape(value or "")
    if kind == "bool":
        return "1" if value else "0"
    return str(value)


def _from_text(kind, text):
    text = text or ""
    if kind == "string":
        return text
    if kind == "bool":
        return text.strip() in ("1", "true")
    if kind == "float":
        return float(text or 0)
    return int(text or 0)


def _from_item(kind, node):
    if kind in _CLASSES:
        return _CLASSES[kind].from_node(node.find("o"))
    return _from_text(kind, node.text)
"""

PY_RUNTIME_TAIL = """

def envelope(msg):
    \"\"\"Returns the bytes to send for the message, including the terminating NUL.\"\"\"
    return (_ENVELOPE_HEAD + msg.NAME + '">' + msg.serialize() + "</cld></protocol>").encode("utf-8") + b"\\0"


def parse(frame):
    \"\"\"Parses one frame (without the NUL), returns the message object or None.\"\"\"
    root = ET.fromstring(frame)
    cld = root if root.tag == "cld" else root.find("cld")
    if cld is None:
        return None
    cls = _CLASSES.get(cld.get("msg", ""))
    if cls is None:
        return None
    return cls.from_node(cld.find("o"))
"""


#
# Main
#
if __name__ == "__main__":
    import argparse
    here = os.path.dirname(os.path.abspath(__file__))
    project = os.path.normpath(os.path.join(here, "..", ".."))
    parser = argparse.ArgumentParser(description="Generates the cloudy protocol code from the IDL")
    parser.add_argument("idl", nargs="?", default=os.path.join(here, "main.idl"))
    parser.add_argument("--cpp-out", default=os.path.join(project, "msg_prot"))
    parser.add_argument("--py-out", default=os.path.join(project, "peer", "cldproto.py"))
    parser.add_argument("--tests", action="store_true", help="also generate the Catch2 tests")
    args = parser.parse_args()

    output_dir = args.cpp_out
    classes = build_structures(args.idl)
    generate(classes)
    print("Structures generated")
    if args.tests:
        generate_tests(classes)
        print("Tests generated")
    generate_message_receiver(classes)
    print("Protocol generated")
    generate_make(classes)
    print("Makefiles generated")
    os.makedirs(os.path.dirname(args.py_out), exist_ok=True)
    generate_python(classes, args.py_out)
    print("Python module generated")

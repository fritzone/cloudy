import os
import random
import string

builtin_types = ["bool", "char", "unsigned char", "short", "unsigned short", "float", "int", "unsigned int", "long",
                 "unsigned long", "double", "long double", "long long", "unsigned long long"]

restrictions = {}

def open_file(fn, mode):
    path = "generated"
    does_exist = os.path.exists(path)
    if not does_exist:
        # Create a new directory because it does not exist
        os.makedirs(path)

    fn_with_path = os.getcwd() + "/generated/" + fn
    return open(fn_with_path, mode)

# returns the 8 character long filename for the given message name, to be used as the header name
# limited to 8.3 under DOS
eight_names = {}
def eight_len_fn(name):
    if name in eight_names:
        print("1. " + name + " -> " + eight_names[name])
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
        print("2. " + name + " -> " + eight_name.lower())
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
                print("3. " + name + " -> " + eight_name.lower())
                return eight_name.lower()
            else:
                name_ctr += 1
                flen -= 1
                if flen == 0:
                    print("Too many identifiers used:", eight_names)
                    exit(2)


# Used for the member declarations
def valid_attribute_type(attr, cls):
    if attr.startswith("string"):
        return "std::string"
    if attr in builtin_types:
        return attr
    if attr.startswith("list"):
        spltd = attr.split()
        if spltd[1] != "of":
            print("Invalid attribute: ", attr, ". Missing 'of' keyword")
            exit(2)
        return "std::vector<" + spltd[2] + ">"
    print("Invalid attribute:", attr, " for ", cls["name"])
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
        return "const std::vector<" + spltd[2] + ">&"
    print("Invalid attribute: ", attr, " for ", cls["name"])
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
            if is_class_definition(attr, classes):
                if attr not in includes:
                    includes.append(attr)
            if attr.startswith("list"):
                spltd = attr.split()
                if spltd[1] != "of":
                    print("Invalid attribute: ", attr, ". Missing 'of' keyword")
                    exit(2)
                if spltd[2] not in includes:
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
            f.write("#include " + eight_len_fn(tbi) + "\n")

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

        # Constructor
        f.write("    " + cls["name"] + "(")
        generate_constructor_init_list(cls, f, pclasses, True)
        # let's see if any of the values are restricted to something
        restricted = False
        opp_written = False
        for a in cls["attributes"]:
            attr = a["type"]
            if attr.find(" of ") != -1:
                restricted = True
                if not opp_written:
                    f.write("\n    {")
                    opp_written = True
                spltd_type = a["type"].split('[')
                if len(spltd_type) != 2:
                    print("Invalid attribute: ", attr, ". Missing [ from the restriction list")
                    exit(2)
                list_of_values = spltd_type[1].split(",")
                if not list_of_values[-1].endswith("]"):
                    print("Invalid attribute: ", attr, ". Missing ] from the restriction list")
                    exit(2)
                if len(list_of_values) > 0:
                    restrictions[cls["name"]][a["name"]] = []
                    f.write("\n        if(")
                    to_join = []
                    for rv in list_of_values:
                        rv = rv.replace("]", "")
                        if not rv.startswith('"') or not rv.endswith('"'):
                            print("Invalid attribute: ", attr, ". String not quoted properly")
                            exit(2)

                        restrictions[cls["name"]][a["name"]].append(rv)
                        to_join.append(a["name"] + " != " + rv)
                    f.write(" && ".join(to_join))
                    f.write(")\n        {\n            ")
                    if a["type"] in builtin_types:
                        f.write(a["name"] + " =  0")
                    else:
                        f.write(a["name"] + " = " + valid_attribute_type(a["type"], cls) + "();")
                    f.write("\n        }")


        if opp_written:
            f.write("\n    }\n")

        if not restricted:
            f.write("\n    {}\n")

        # Now the setters for the attributes
        f.write("\n    // setters\n")
        if len(cls["attributes"]) > 0:
            attr_names = [d.get("name", '') for d in cls["attributes"]]
            attr_types = [d.get("type", '') for d in cls["attributes"]]
            for a, t in zip(attr_names, attr_types):
                f.write("    void set_" + a + "(" + const_if_can_attribute_type(t, cls) + " p_" + a + ");\n")

        # Now the getters for the attributes
        f.write("\n    // getters\n")
        if len(cls["attributes"]) > 0:
            attr_names = [d.get("name", '') for d in cls["attributes"]]
            attr_types = [d.get("type", '') for d in cls["attributes"]]
            for a, t in zip(attr_names, attr_types):
                f.write("    " + valid_attribute_type(t, cls) + " get_" + a + "() const\n    {\n")
                f.write("        return " + a + ";\n")
                f.write("    }\n")

        # serializer
        f.write("\n    // serializer\n")
        f.write("    std::string serialize();\n")
        f.write("    int deserialize(const char*);\n")

        # The equality operator
        f.write("\n    // comparison\n")
        f.write('    bool operator == (const ' + cls["name"] + "&) const;\n")

        # Now the attributes

        f.write("\nprivate:\n")
        if len(cls["attributes"]) > 0:
            attr_names = [d.get("name", '') for d in cls["attributes"]]
            attr_types = [d.get("type", '') for d in cls["attributes"]]
            for a, t in zip(attr_names, attr_types):
                f.write("    " + valid_attribute_type(t, cls) + " " + a + ";\n" )

        f.write("};\n")
        f.write("#endif\n")
        f.close()

        # Then the cpp
        f = open_file(eight_len_fn(cls["name"]) + ".cpp", "w")
        f.write("#include \"" + eight_len_fn(cls["name"]) + ".h\"\n")
        f.write("#include \"strngify.h\"\n\n")
        f.write("#include \"ezxml.h\"\n\n")
        f.write("#include <string.h>\n\n")

        # The serializer function
        f.write('std::string ' + cls["name"] + '::serialize()\n{\n')
        f.write('    std::string result = "<o><type>' + cls["name"] + '</type>";\n')
        if len(cls["attributes"]) > 0:
            f.write('    result += "<attributes>";\n')
            for a in cls["attributes"]:
                f.write("    // attribute:" + a["name"] + "\n")
                f.write('    result += "<' + a["name"] + '>";\n')
                if a["type"] in builtin_types or a["type"].startswith("string"):
                    f.write('    result += stringify(' + a["name"] + ');')
                else:
                    f.write('    for(int i=0; i<' + a["name"] + '.size(); i++) result += "<item i=\\"" + stringify(i) + "\\">" + ' + a["name"] + '[i].serialize() + "</item>\";')
                f.write('\n    result += "</' + a["name"] + '>";\n')
            f.write('    result += "</attributes>";\n')
        f.write('    result += "</o>";\n')
        f.write('    return result;\n}\n')

        # The deserializer function
        f.write('int ' + cls["name"] + '::deserialize(const char* xml)\n{\n')
        f.write('    ezxml_t x = ezxml_parse_str((char*)xml, strlen(xml));\n')
        f.write('    ezxml_t type_node = ezxml_child(x, "type");\n')
        f.write('    const char* str_type = type_node->txt;\n')
        f.write('    if(strcmp(str_type, "' + cls["name"] + '")) return 0;')
        if len(cls["attributes"]) > 0:
            f.write('    ezxml_t attrs_node = ezxml_child(x, "attributes");\n')
            f.write('    if(!attrs_node) return 0;\n')
            for a in cls["attributes"]:
                f.write('    ezxml_t attr_node_' + a["name"] + ' = ezxml_child(attrs_node, "' + a["name"] + '");\n')
                if a["type"] in builtin_types or a["type"].startswith("string"):
                    f.write('    destringify(' + a["name"] + ', attr_node_' + a["name"] + '->txt);\n')
                else:
                    f.write('    for (ezxml_t item = ezxml_child(attr_node_' + a["name"] + ', "item"); item; item = item->next) {\n')
                    tdls = a["type"].split()
                    if tdls[0] != 'list' and tdls[1] != 'of':
                        print("Invalid attribute: ", a["type"], ". Missing something from it ...")
                        exit(2)
                    f.write('        ' + tdls[2] + ' l_' + tdls[2] + ';\n')
                    f.write('        ezxml_t items_o = ezxml_child(item, "o");\n')
                    f.write('        char * l_attr_data = ezxml_toxml(items_o);\n')
                    f.write('        if(!l_' + tdls[2] + '.deserialize(l_attr_data))  {free(l_attr_data); return 0;}\n')
                    f.write('        free(l_attr_data);\n')
                    f.write('        ' + a["name"] + '.push_back(l_' + tdls[2] + ');\n')


                    f.write('\n    }')

        f.write('\n    return 1;\n}\n')

        # Comparison operator implemented
        f.write('bool ' + cls["name"] + '::operator == (const ' + cls["name"] + "& rhs) const\n{\n")
        for e in cls["extends"]:
            f.write("    if(!" + e + "::operator ==(rhs)) return false;\n")
        for a in cls["attributes"]:
            f.write("    if(" + a["name"] + " != " + "rhs." + a["name"] + ") return false;\n")

        f.write('\n    return true;\n}\n')

        # setter implemented
        if len(cls["attributes"]) > 0:
            attr_names = [d.get("name", '') for d in cls["attributes"]]
            attr_types = [d.get("type", '') for d in cls["attributes"]]
            for a, t in zip(attr_names, attr_types):
                f.write("void " + cls["name"]+ "::set_" + a + "(" + const_if_can_attribute_type(t, cls) + " p_" + a + ")\n")
                f.write("{")
                if a in restrictions[cls["name"]]:
                    to_join = []
                    for r in restrictions[cls["name"]][a]:
                        to_join.append("p_" + a + " != " + r)
                    f.write("\n    if (" + " && ".join(to_join) + ") { return; }")
                f.write("\n    " + a + " = p_" + a + ";")
                f.write("\n}\n")


        f.close()


# will generate a constructor, default_v is whether there is a default value or not
def generate_constructor_init_list(cls, f, pclasses, default_v):
    attrs = []
    for a in all_attributes(cls, pclasses):
        cpar = const_if_can_attribute_type(a["type"], cls) + " p_" + a["name"]
        if default_v:
            if a["type"] in builtin_types:
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
        attr_names = [d.get("name", '') for d in cls["attributes"]]
        attr_par_values = ['p_' + element for element in attr_names]
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


def build_structures(filename):

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
    f.write("\n)")
    f.close()

    mf = open_file("makefile", "w")
    mf.write("protocol_objs = ")
    to_join = []
    for cls in pclasses:
        to_join.append(eight_len_fn(cls["name"]) + ".o")
    mf.write(" ".join(to_join) + "\n")
    mf.close()



#
# Will generate some test cases. Just in case.
#
def generate_tests(pclasses):
    path = "generated"
    does_exist = os.path.exists(path)
    if not does_exist:
        # Create a new directory because it does not exist
        os.makedirs(path)

    path = "generated/tests"
    # Check whether the specified path exists or not
    does_exist = os.path.exists(path)
    if not does_exist:
        # Create a new directory because it does not exist
        os.makedirs(path)

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
# Main
#
if __name__ == "__main__":
    classes = build_structures("main.idl")
    generate(classes)
    generate_tests(classes)
    generate_make(classes)

    print(restrictions)

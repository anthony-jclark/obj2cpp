// TODO
// - add checks for bad fstream state
// - make more general (optional arguments)
// - optional output to file

#include <iostream>
#include <fstream>
#include <sstream>

#include <iterator>
#include <algorithm>

#include <vector>
#include <string>
using std::string;


// A simple struct for holding three double values
struct vec3f { double v1, v2, v3; };
using read_vec3f = std::istream_iterator<vec3f>;
std::istream& operator>>(std::istream& in, vec3f& v) {
    return in >> v.v1 >> v.v2 >> v.v3;
}
std::ostream& operator<<(std::ostream& out, const vec3f& v) {
    return out << v.v1 << "f, " << v.v2 << "f, " << v.v3 << "f";
}

// A simple structor for holding two double values
struct vec2f { double v1, v2; };
using read_vec2f = std::istream_iterator<vec2f>;
std::istream& operator>>(std::istream& in, vec2f& v) {
    return in >> v.v1 >> v.v2;
}
std::ostream& operator<<(std::ostream& out, const vec2f& v) {
    return out << v.v1 << "f, " << v.v2 << "f";
}

struct vec3i { size_t i1, i2, i3; };
using read_vec3i = std::istream_iterator<vec3i>;
std::istream& operator>>(std::istream& in, vec3i& v) {
    char sep1, sep2;
    return in >> v.i1 >> sep1 >> v.i2 >> sep2 >> v.i3;
}

struct object
{
    string path, name;
    std::vector<vec3f> positions;
    std::vector<vec2f> texels;
    std::vector<vec3f> normals;
    std::vector<vec3i> faces;
};

object read_obj_file(string fname)
{
    std::ifstream obj_file(fname);
    if (!obj_file.is_open()) {
        std::cerr << "Could not open file: " << fname << std::endl;
        exit(EXIT_FAILURE);
    }

    // Read data into an object
    object obj;
    obj.path = fname;
    auto idx1 = fname.find_last_of("\\/");
    auto idx2 = fname.find_last_of(".");
    obj.name = fname.substr(idx1 + 1, idx2 - idx1 - 1);

    string line, line_tag;
    while (getline(obj_file, line))
    {
        std::istringstream iss(line);
        iss >> line_tag;
        if (line_tag == "v") {
            // Read (x,y,z[,w]), w default to 1.0
            std::copy(read_vec3f(iss), read_vec3f(), std::back_inserter(obj.positions));
        } else if (line_tag == "vt") {
            // Read (u,v[,w]), w defaults to 0
            std::copy(read_vec2f(iss), read_vec2f(), std::back_inserter(obj.texels));
        } else if (line_tag == "vn") {
            // Read (x,y,z), may need to be normalized
            std::copy(read_vec3f(iss), read_vec3f(), std::back_inserter(obj.normals));
        } else if (line_tag == "f") {
            // v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ...
            // indices start at 1
            // negative indices index from end of list
            // texture and normal indices are optional
            std::copy(read_vec3i(iss), read_vec3i(), std::back_inserter(obj.faces));
        } else {
            std::cerr << "unhandled tag : " << line_tag << std::endl;
        }
    }
    return obj;
}


void write_header(std::ostream& out, object& obj)
{
    const size_t TAB_SIZE = 4;
    const string TAB(TAB_SIZE, ' ');

    string guard("_");
    std::transform(begin(obj.name), end(obj.name), std::back_inserter(guard), ::toupper);
    guard += "_HEADER_";

    // Write the top of the file
    out << "// This file generated from : " << obj.path << std::endl;
    out << "\n#ifndef " << guard;
    out << "\n#define " << guard << "\n\n";
    out << "struct " << obj.name << "_OBJ\n{\n";

    // Add a member for the number of vertices
    out << TAB << "static const size_t num_verts = " << obj.faces.size() << ";\n";

    // Add the vertex positions member variable
    out << TAB << "static const float positions = {\n";
    string sep = "";
    for (const auto &f : obj.faces) {
        out << sep << TAB << TAB << obj.positions.at(f.i1 - 1);
        sep = ",\n";
    }
    out << "\n" << TAB << "};\n";

    // Add the texels member variable
    out << TAB << "static const float texels = {\n";
    sep = "";
    for (const auto &f : obj.faces) {
        out << sep << TAB << TAB << obj.texels.at(f.i2 - 1);
        sep = ",\n";
    }
    out << "\n" << TAB << "};\n";

    // Add the normals member variable
    out << TAB << "static const float normals = {\n";
    sep = "";
    for (const auto &f : obj.faces) {
        out << sep << TAB << TAB << obj.normals.at(f.i3 - 1);
        sep = ",\n";
    }
    out << "\n" << TAB << "};\n";

    // Write the bottom of the file
    out << "};\n\n#endif\n";

}


int main(int argc, char const *argv[])
{
    if (argc < 2) {
        std::cerr << "Must provide an input file." << std::endl;
        return EXIT_FAILURE;
    }
    string obj_name = argv[1];

    // Read the OBJ file
    auto obj = read_obj_file(obj_name);

    // std::cout << "vertices : ";
    // for (const auto& p : obj.positions) {
    //     std::cout << "{" << p.v1 << "," << p.v2 << "," << p.v3 << "}";
    // }

    write_header(std::cout, obj);


    return 0;
}
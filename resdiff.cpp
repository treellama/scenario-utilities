/*
    resdiff: generates MML describing the resource fork differences between two 
        MacBinary-encoded Marathon Infinity-derived engines
    Copyright (C) 2022 Gregory Smith

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <array>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include <boost/crc.hpp>
#include <boost/endian/arithmetic.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "macroman.h"

using namespace boost::endian;
namespace pt = boost::property_tree;

using ResourceType = std::array<char, 4>;
using ResourceId = std::pair<ResourceType, int>;

struct ResourceForkHeader {
    big_uint32_t data_offset;
    big_uint32_t map_offset;
    big_uint32_t data_length;
    big_uint32_t map_length;
};

struct TypeListEntry {
    ResourceType type;
    big_int16_t num_refs;
    big_int16_t ref_list_offset;
};

struct RefListEntry {
    big_int16_t id;
    big_int16_t name_list_offset;
    big_uint32_t data_offset;
    big_uint32_t unused;
};

struct RGBColor {
    pt::ptree diff(const RGBColor& other) {
        pt::ptree tree;

        if (r != other.r ||
            g != other.g ||
            b != other.b)
        {
            tree.put("color.<xmlattr>.red", other.r / 65535.0);
            tree.put("color.<xmlattr>.green", other.g / 65535.0);
            tree.put("color.<xmlattr>.blue", other.b / 65535.0);
        }

        return tree;
    }

    pt::ptree diff(int index, const RGBColor& other) {
        pt::ptree tree;

        if (r != other.r ||
            g != other.g ||
            b != other.b)
        {
            tree.put("color.<xmlattr>.index", index);
            tree.put("color.<xmlattr>.red", other.r / 65535.0);
            tree.put("color.<xmlattr>.green", other.g / 65535.0);
            tree.put("color.<xmlattr>.blue", other.b / 65535.0);
        }

        return tree;
    }
    
    big_uint16_t r;
    big_uint16_t g;
    big_uint16_t b;
};

bool operator==(const RGBColor& a, const RGBColor& b)
{
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

bool operator!=(const RGBColor& a, const RGBColor& b)
{
    return !(a == b);
}

struct Rect {
    pt::ptree diff(int index, const Rect& other) {
        pt::ptree tree;

        if (top != other.top ||
            left != other.left ||
            bottom != other.bottom ||
            right != other.right)
        {
            tree.put("rect.<xmlattr>.index", index);
            tree.put("rect.<xmlattr>.top", other.top);
            tree.put("rect.<xmlattr>.left", other.left);
            tree.put("rect.<xmlattr>.bottom", other.bottom);
            tree.put("rect.<xmlattr>.right", other.right);
        }

        return tree;
    }
    
    big_uint16_t top;
    big_uint16_t left;
    big_uint16_t bottom;
    big_uint16_t right;
};

bool operator==(const Rect& a, const Rect& b)
{
    return a.top == b.top && a.left == b.left &&
        a.bottom == b.bottom && a.right == b.right;
}

bool operator!=(const Rect& a, const Rect& b) {
    return !(a == b);
}

class MacBinary {
public:
    class Exception : public std::runtime_error {
    public:
        Exception(const char* what) : std::runtime_error{what} { }
    };
    
    MacBinary(const char* filename) : stream_{filename} {
        load();
    }

    const std::vector<uint8_t>& GetResource(uint32_t type, int16_t id);

    void diff(MacBinary& other);

private:
    void load();
    void load_resources();

    std::map<int, std::vector<std::string>> strings_;
    std::array<RGBColor, 26> interface_colors_;
    std::array<Rect, 18> interface_rects_;

    std::ifstream stream_;
};

void MacBinary::load()
{
    std::array<uint8_t, 128> header;
    try {
        stream_.read(reinterpret_cast<char*>(header.data()), header.size());
    } catch (std::ios_base::failure) {
        throw Exception("File not long enough");
    }

    if (header[0] || header[1] > 63 || header[74] || header[123] > 0x81) {
        throw Exception("Header magic mismatch");
    }
    
    boost::crc_optimal<16, 0x1021, 0, 0, false, false> crc;
    crc.process_bytes(header.data(), 124);
    if (crc.checksum() != ((header[124] << 8) | header[125])) {
        throw Exception("Header CRC mismatch");
    }

    big_uint32_t data_length;
    std::copy_n(&header[83], 4, reinterpret_cast<uint8_t*>(&data_length));

    big_uint32_t resource_length;
    std::copy_n(&header[87], 4, reinterpret_cast<uint8_t*>(&resource_length));

    uint32_t resource_offset = 128 + ((data_length.value() + 0x7f) & ~0x7f);

    stream_.seekg(resource_offset, stream_.beg);
    load_resources();
}

void MacBinary::load_resources()
{
    std::streampos start = stream_.tellg();

    ResourceForkHeader header;
    stream_.read(reinterpret_cast<char*>(&header), 16);

    header.data_offset += start;
    header.map_offset += start;

    stream_.seekg(header.map_offset.value() + 24);

    big_uint16_t type_list_offset;
    stream_.read(reinterpret_cast<char*>(&type_list_offset), 2);

    type_list_offset += header.map_offset.value();

    stream_.seekg(2, stream_.cur);

    big_uint16_t num_types;
    stream_.read(reinterpret_cast<char*>(&num_types), 2);
    ++num_types;

    std::vector<TypeListEntry> type_list(num_types);
    stream_.read(reinterpret_cast<char*>(type_list.data()), num_types * 8);

    std::map<int16_t, uint32_t> str_offsets;
    uint32_t clut_130_offset = 0;
    uint32_t nrct_128_offset = 0;

    for (auto& type_list_entry : type_list) {
        if (type_list_entry.type == ResourceType{'S','T','R','#'}) {
            for (auto i = 0; i < type_list_entry.num_refs + 1; ++i) {
                RefListEntry ref_list_entry;
                stream_.read(reinterpret_cast<char*>(&ref_list_entry), 12);
                str_offsets[ref_list_entry.id] = ref_list_entry.data_offset & 0x00ffffff;
            }
        } else if (type_list_entry.type == ResourceType{'c','l','u','t'}) {
            for (auto i = 0; i < type_list_entry.num_refs + 1; ++i) {
                RefListEntry ref_list_entry;
                stream_.read(reinterpret_cast<char*>(&ref_list_entry), 12);
                if (ref_list_entry.id == 130) {
                    clut_130_offset = ref_list_entry.data_offset & 0x00ffffff;
                }
            }
        } else if (type_list_entry.type == ResourceType{'n','r','c','t'}) {
            for (auto i = 0; i < type_list_entry.num_refs + 1; ++i) {
                RefListEntry ref_list_entry;
                stream_.read(reinterpret_cast<char*>(&ref_list_entry), 12);
                if (ref_list_entry.id == 128) {
                    nrct_128_offset = ref_list_entry.data_offset & 0x00ffffff;
                }
            }
        } else {
            stream_.seekg((type_list_entry.num_refs + 1) * 12, stream_.cur);
        }
    }

    for (auto& str : str_offsets) {
        stream_.seekg(str.second + header.data_offset);

        big_uint32_t length;
        stream_.read(reinterpret_cast<char*>(&length), 4);

        big_int16_t num_strings;
        stream_.read(reinterpret_cast<char*>(&num_strings), 2);

        auto& v = strings_[str.first];
        v.clear();
        
        for (auto i = 0; i < num_strings; ++i) {
            uint8_t string_length;
            stream_.read(reinterpret_cast<char*>(&string_length), 1);

            std::string s;
            s.resize(string_length);
            stream_.read(&s[0], s.size());

            v.push_back(s);
        }
    }

    {
        // clut id 130 sets interface colors
        stream_.seekg(clut_130_offset + header.data_offset);

        big_uint32_t length;
        stream_.read(reinterpret_cast<char*>(&length), 4);

        big_uint32_t seed;
        stream_.read(reinterpret_cast<char*>(&length), 4);

        big_uint16_t flags;
        stream_.read(reinterpret_cast<char*>(&flags), 2);

        big_uint16_t num_colors;
        stream_.read(reinterpret_cast<char*>(&num_colors), 2);

        if (num_colors != 25) {
            std::ostringstream oss;
            oss << "Unexpected number colors in clut 130: " << num_colors;
            throw std::runtime_error(oss.str());
        }

        for (auto i = 0; i < num_colors; ++i) {
            big_uint16_t pixel_value;
            stream_.read(reinterpret_cast<char*>(&pixel_value), 2);
            stream_.read(reinterpret_cast<char*>(&interface_colors_[i]), 6);
        }
    }

    {
        // ntct 128 sets interface rectangles
        stream_.seekg(nrct_128_offset + header.data_offset);

        big_uint32_t length;
        stream_.read(reinterpret_cast<char*>(&length), 4);

        big_uint16_t num_rects;
        stream_.read(reinterpret_cast<char*>(&num_rects), 2);

        if (num_rects != 18) {
            std::ostringstream oss;
            oss << "Unexpected number of colors in nrct 128: " << num_rects;
            throw std::runtime_error(oss.str());
        }

        for (auto i = 0; i < num_rects; ++i) {
            stream_.read(reinterpret_cast<char*>(&interface_rects_[i]), 8);
        }
    }
        
}

void MacBinary::diff(MacBinary& other)
{
    pt::ptree tree;
    tree.add("<xmlcomment>", "Generated by resdiff");
    for (auto& id : strings_) {
        if (id.first == 129) {
            // skip filenames
            continue;
        }
        auto found_diff = false;
        pt::ptree stringset_tree;

        stringset_tree.put("stringset.<xmlattr>.index", id.first);
        
        if (id.second.size() != other.strings_[id.first].size()) {
            std::ostringstream oss;
            oss << "Not yet implemented: different num strings for id " << id.first;
            throw std::runtime_error(oss.str());
        } else {
            auto& v = id.second;
            auto& other_v = other.strings_[id.first];
            for (auto i = 0; i < v.size(); ++i) {
                if (v[i] != other_v[i]) {
                    pt::ptree string_tree;

                    string_tree.put("string", mac_roman_to_utf8(other_v[i]));
                    string_tree.put("string.<xmlattr>.index", i);

                    found_diff = true;

                    stringset_tree.add_child("stringset.string", string_tree.get_child("string"));
                }
            }
        }

        if (found_diff) {
            tree.add_child("marathon.stringset", stringset_tree.get_child("stringset"));
        }
    }

    for (auto i = 0; i < 25; ++i) {
        if (interface_colors_[i] != other.interface_colors_[i]) {
            auto color_tree = interface_colors_[i].diff(i, other.interface_colors_[i]);
            tree.add_child("marathon.interface.color", color_tree.get_child("color"));
        }
    }

    for (auto i = 0; i < 18; ++i) {
        if (interface_rects_[i] != other.interface_rects_[i]) {
            auto rect_tree = interface_rects_[i].diff(i, other.interface_rects_[i]);
            tree.add_child("marathon.interface.rect", rect_tree.get_child("rect"));
        }
    }

    pt::xml_writer_settings<std::string> settings(' ', 4, "utf-8");
    pt::write_xml(std::cout, tree, settings);
}

int main(int argv, char* argc[])
{
    if (argv != 3) {
        std::cerr << "Usage: strdiff <base> <modified>\n";
        return- 1;
    }

    try {
        MacBinary base{argc[1]};
        MacBinary mod{argc[2]};

        base.diff(mod);
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return -1;
    }

    return 0;
}

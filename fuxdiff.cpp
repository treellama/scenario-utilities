/*
    fuxdiff: creates MML describing the difference between two Fux! state files
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
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include <boost/endian/arithmetic.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace boost::endian;
namespace pt = boost::property_tree;

using Tag = std::array<char, 4>;

using big_fixed_t = big_int32_t;

std::ostream& operator<<(std::ostream& s, Tag tag) {
    for (auto c : tag) {
        s << c;
    }

    return s;
}

struct Header {
    Tag tag;
    big_uint32_t length;
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

struct AnnotationDefinition {
    RGBColor color;
    big_int16_t font;
    big_int16_t face;
    std::array<big_int16_t, 4> sizes;
};

struct ControlPanelDefinition {
    pt::ptree diff(int index, const ControlPanelDefinition& other) {
        pt::ptree tree;

        if (panel_class != other.panel_class ||
            flags != other.flags ||
            collection != other.collection ||
            active_shape != other.active_shape ||
            inactive_shape != other.inactive_shape ||
            sounds != other.sounds ||
            sound_frequency != other.sound_frequency ||
            item != other.item)
        {
            tree.put("panel.<xmlattr>.index", index);
            tree.put("panel.<xmlattr>.type", other.panel_class);
            tree.put("panel.<xmlattr>.coll", other.collection);
            tree.put("panel.<xmlattr>.active_frame", other.active_shape);
            tree.put("panel.<xmlattr>.inactive_frame", other.inactive_shape);
            tree.put("panel.<xmlattr>.pitch", other.sound_frequency / 65536.0);
            tree.put("panel.<xmlattr>.item", other.item);
            for (auto i = 0; i < sounds.size(); ++i) {
                if (sounds[i] != other.sounds[i]) {
                    pt::ptree sound_tree;
                    sound_tree.put("sound.<xmlattr>.type", i);
                    sound_tree.put("sound.<xmlattr>.which", other.sounds[i]);

                    tree.add_child("panel.sound", sound_tree.get_child("sound"));
                }
            }
        }

        return tree;
    }
    
    big_int16_t panel_class;
    big_uint16_t flags;

    big_int16_t collection;
    big_int16_t active_shape, inactive_shape;

    std::array<big_int16_t, 3> sounds;
    big_fixed_t sound_frequency;

    big_int16_t item;
};

struct DamageDefinition {
    pt::ptree diff(const DamageDefinition& other) {
        pt::ptree tree;

        if (type != other.type ||
            flags != other.flags ||
            base != other.base ||
            random != other.random ||
            scale != other.scale)
        {
            tree.put("damage.<xmlattr>.type", other.type);
            tree.put("damage.<xmlattr>.flags", other.flags);
            tree.put("damage.<xmlattr>.base", other.base);
            tree.put("damage.<xmlattr>.random", other.random);
            tree.put("damage.<xmlattr>.scale", other.scale / 65536.0);
        }

        return tree;
    };
    big_int16_t type;
    big_int16_t flags;
    big_int16_t base;
    big_int16_t random;
    big_fixed_t scale;
};

struct DamageResponse {
    pt::ptree diff(const DamageResponse& other, int index) {
        pt::ptree tree;

        assert(type == other.type);
        
        if (threshold != other.threshold ||
            fade != other.fade ||
            sound != other.sound ||
            death_sound != other.death_sound ||
            death_action != other.death_action)
        {
            tree.put("damage.<xmlattr>.index", index);
            tree.put("damage.<xmlattr>.threshold", other.threshold);
            tree.put("damage.<xmlattr>.fade", other.fade);
            tree.put("damage.<xmlattr>.sound", other.sound);
            tree.put("damage.<xmlattr>.death_sound", other.death_sound);
            tree.put("damage.<xmlattr>.death_action", other.death_action);
        }
            
        return tree;
    }
    
    big_int16_t type;
    big_int16_t threshold;

    big_int16_t fade;
    big_int16_t sound;
    big_int16_t death_sound;
    big_int16_t death_action;
};

struct FadeDefinition {
    pt::ptree diff(int index, const FadeDefinition& other) {
        pt::ptree tree;

        if (proc != other.proc ||
            color != other.color ||
            initial_transparency != other.initial_transparency ||
            final_transparency != other.final_transparency ||
            period != other.period ||
            flags != other.flags ||
            priority != other.priority)
        {
            tree.put("fader.<xmlattr>.index", index);
            tree.put("fader.<xmlattr>.type", other.proc);
            tree.put("fader.<xmlattr>.initial_opacity",
                     other.initial_transparency / 65536.0);
            tree.put("fader.<xmlattr>.final_opacity",
                     other.final_transparency / 65536.0);
            tree.put("fader.<xmlattr>.period", other.period);
            tree.put("fader.<xmlattr>.flags", other.flags);
            tree.put("fader.<xmlattr>.priority", other.priority);

            auto color_tree = color.diff(other.color);
            if (!color_tree.empty()) {
                tree.add_child("fader.color", color_tree.get_child("color"));
            }
        }

        return tree;
    }
    
    big_uint32_t proc;
    RGBColor color;
    big_fixed_t initial_transparency, final_transparency;
    big_int16_t period;
    big_uint16_t flags;
    big_int16_t priority;
};

struct LineDefinition {
    pt::ptree diff(int index, const LineDefinition& other) {
        pt::ptree tree;

        if (color != other.color ||
            pen_sizes != other.pen_sizes)
        {
            auto color_tree = color.diff(other.color);
            if (!color_tree.empty()) {
                tree.add_child("line.color", color_tree.get_child("color"));
            }

            for (auto i = 0; i < pen_sizes.size(); ++i) {
                if (pen_sizes[i] != other.pen_sizes[i]) {
                    
                }
            }
        }

        return tree;
    }
    RGBColor color;
    std::array<big_int16_t, 4> pen_sizes;
};

struct MediaDefinition {
    pt::ptree diff(int index, const MediaDefinition& other) {
        pt::ptree tree;
        
        auto damage_tree = damage.diff(other.damage);
        if (collection != other.collection ||
            shape != other.shape ||
            shape_count != other.shape_count ||
            /* shape_frequency is unused */
            transfer_mode != other.transfer_mode ||
            damage_frequency != other.damage_frequency ||
            !damage_tree.empty() ||
            detonation_effects != other.detonation_effects ||
            sounds != other.sounds ||
            submerged_fade_effect != other.submerged_fade_effect)
        {
            tree.put("liquid.<xmlattr>.index", index);
            tree.put("liquid.<xmlattr>.coll", other.collection);
            tree.put("liquid.<xmlattr>.frame", other.shape);
            tree.put("liquid.<xmlattr>.transfer", other.transfer_mode);
            tree.put("liquid.<xmlattr>.damage_freq", other.damage_frequency);
            if (!damage_tree.empty()) {
                 tree.add_child("liquid.damage", damage_tree.get_child("damage"));
            }
            for (auto i = 0; i < detonation_effects.size(); ++i) {
                // TODO: should we always include these? or only when different?
                if (detonation_effects[i] != other.detonation_effects[i]) {
                    pt::ptree effect_tree;
                    effect_tree.put("effect.<xmlattr>.type", i);
                    effect_tree.put("effect.<xmlattr>.which", other.detonation_effects[i]);
                    tree.add_child("liquid.effect", effect_tree.get_child("effect"));
                }
            }
            for (auto i = 0; i < sounds.size(); ++i) {
                // TODO: should we always include these? or only when different?
                if (sounds[i] != other.sounds[i]) {
                    pt::ptree sound_tree;
                    sound_tree.put("sound.<xmlattr>.type", i);
                    sound_tree.put("sound.<xmlattr>.which", other.sounds[i]);
                    tree.add_child("liquid.sound", sound_tree.get_child("sound"));
                }
            }
            tree.put("liquid.<xmlattr>.submerged", other.submerged_fade_effect);
        }

        return tree;
    };
    
    big_int16_t collection;
    big_int16_t shape;
    big_int16_t shape_count;
    big_int16_t shape_frequency;
    big_int16_t transfer_mode;
    big_int16_t damage_frequency;
    DamageDefinition damage;

    std::array<big_int16_t, 4> detonation_effects;
    std::array<big_int16_t, 9> sounds;
    big_int16_t submerged_fade_effect;
};

struct SceneryDefinition {
    pt::ptree diff(int index, const SceneryDefinition& other) {
        pt::ptree tree;
        if (flags != other.flags ||
            shape != other.shape ||
            radius != other.radius ||
            height != other.height ||
            destroyed_effect != other.destroyed_effect ||
            destroyed_shape != other.destroyed_shape)
        {
            tree.put("object.<xmlattr>.index", index);
            tree.put("object.<xmlattr>.flags", other.flags);
            tree.put("object.<xmlattr>.radius", other.radius);
            tree.put("object.<xmlattr>.height", other.height);
            tree.put("object.<xmlattr>.destruction", other.destroyed_effect);

            if (shape != other.shape) {
                pt::ptree child;
                child.put("shape.<xmlattr>.coll", (other.shape >> 8) & 0x1f);
                child.put("shape.<xmlattr>.clut", other.shape >> 11);
                child.put("shape.<xmlattr>.seq", other.shape & 0xff);

                tree.add_child("object.normal.shape", child.get_child("shape"));
            }

            if (destroyed_shape != other.destroyed_shape) {
                pt::ptree child;
                child.put("shape.<xmlattr>.coll", (other.destroyed_shape >> 8) & 0x1f);
                child.put("shape.<xmlattr>.clut", (other.destroyed_shape >> 11));
                child.put("shape.<xmlattr>.seq", (other.destroyed_shape & 0xff));

                tree.add_child("object.destroyed.shape", child.get_child("shape"));
            }
        }

        return tree;
    }
    
    big_uint16_t flags;
    big_uint16_t shape;

    big_int16_t radius, height;

    big_int16_t destroyed_effect;
    big_uint16_t destroyed_shape;
};

struct WeaponInterfaceAmmoDefinition {
    pt::ptree diff(int index, const WeaponInterfaceAmmoDefinition& other) {
        pt::ptree tree;

        if (type != other.type ||
            screen_left != other.screen_left ||
            screen_top != other.screen_top ||
            ammo_across != other.ammo_across ||
            ammo_down != other.ammo_down ||
            delta_x != other.delta_x ||
            delta_y != other.delta_y ||
            bullet != other.bullet ||
            empty_bullet != other.empty_bullet ||
            right_to_left != other.right_to_left)
        {
            tree.put("ammo.<xmlattr>.index", index);
            tree.put("ammo.<xmlattr>.type", other.type);
            tree.put("ammo.<xmlattr>.left", other.screen_left);
            tree.put("ammo.<xmlattr>.top", other.screen_top);
            tree.put("ammo.<xmlattr>.across", other.ammo_across);
            tree.put("ammo.<xmlattr>.down", other.ammo_down);
            tree.put("ammo.<xmlattr>.delta_x", other.delta_x);
            tree.put("ammo.<xmlattr>.delta_y", other.delta_y);
            tree.put("ammo.<xmlattr>.bullet_shape", other.bullet);
            tree.put("ammo.<xmlattr>.empty_shape", other.empty_bullet);
            tree.put("ammo.<xmlattr>.right_to_left", other.right_to_left != 0);
        }

        return tree;
    }
    
    big_int16_t type;
    big_int16_t screen_left;
    big_int16_t screen_top;
    big_int16_t ammo_across;
    big_int16_t ammo_down;
    big_int16_t delta_x;
    big_int16_t delta_y;
    big_int16_t bullet;
    big_int16_t empty_bullet;
    big_uint16_t right_to_left;
};

bool operator==(const WeaponInterfaceAmmoDefinition& a, const WeaponInterfaceAmmoDefinition& b)
{
    return a.type == b.type &&
        a.screen_left == b.screen_left &&
        a.screen_top == b.screen_top &&
        a.ammo_across == b.ammo_across &&
        a.ammo_down == b.ammo_down &&
        a.delta_x == b.delta_x &&
        a.delta_y == b.delta_y &&
        a.bullet == b.bullet &&
        a.empty_bullet == b.empty_bullet &&
        a.right_to_left == b.right_to_left;
}

bool operator!=(const WeaponInterfaceAmmoDefinition& a, const WeaponInterfaceAmmoDefinition& b)
{
    return !(a == b);
}

struct WeaponInterfaceDefinition {
    pt::ptree diff(int index, const WeaponInterfaceDefinition& other) {
        pt::ptree tree;
        if (item_id != other.item_id) {
            std::cerr << "Weapon HUD items changed; Aleph One does not support this!\n";
        }
        
        if (weapon_panel_shape != other.weapon_panel_shape ||
            weapon_name_start_y != other.weapon_name_start_y ||
            weapon_name_end_y != other.weapon_name_end_y ||
            weapon_name_start_x != other.weapon_name_start_x ||
            weapon_name_end_x != other.weapon_name_end_x ||
            standard_weapon_panel_top != other.standard_weapon_panel_top ||
            standard_weapon_panel_left != other.standard_weapon_panel_left ||
            multi_weapon != other.multi_weapon ||
            ammo_data[0] != other.ammo_data[0] ||
            ammo_data[1] != other.ammo_data[1])
        {
            tree.put("weapon.<xmlattr>.index", index);
            tree.put("weapon.<xmlattr>.shape", other.weapon_panel_shape);
            tree.put("weapon.<xmlattr>.start_y", other.weapon_name_start_y);
            tree.put("weapon.<xmlattr>.end_y", other.weapon_name_end_y);
            tree.put("weapon.<xmlattr>.start_x", other.weapon_name_start_x);
            tree.put("weapon.<xmlattr>.end_x", other.weapon_name_end_x);
            tree.put("weapon.<xmlattr>.top", other.standard_weapon_panel_top);
            tree.put("weapon.<xmlattr>.left", other.standard_weapon_panel_left);
            tree.put("weapon.<xmlattr>.multiple", other.multi_weapon != 0);

            for (auto i = 0; i < 2; ++i) {
                auto ammo_tree = ammo_data[i].diff(i, other.ammo_data[i]);
                if (!ammo_tree.empty()) {
                    tree.add_child("weapon.ammo", ammo_tree.get_child("ammo"));
                }
            }
        }

        return tree;
    }
    
    big_int16_t item_id;
    big_int16_t weapon_panel_shape;
    big_int16_t weapon_name_start_y;
    big_int16_t weapon_name_end_y;
    big_int16_t weapon_name_start_x;
    big_int16_t weapon_name_end_x;
    big_int16_t standard_weapon_panel_top;
    big_int16_t standard_weapon_panel_left;
    big_uint16_t multi_weapon;

    WeaponInterfaceAmmoDefinition ammo_data[2];
};

class Fuxstate {
public:
    void diff(Fuxstate& other);
    void load(const char* filename);
    void load(std::istream& s);

    AnnotationDefinition annotation_definition;
    std::array<ControlPanelDefinition, 54> control_panels;
    std::array<DamageResponse, 24> damage_responses;
    std::array<FadeDefinition, 32> fade_definitions;
    std::array<RGBColor, 4> infravision_colors;
    std::array<LineDefinition, 3> line_definitions;
    RGBColor map_name_color;
    std::array<MediaDefinition, 5> media_definitions;
    std::array<RGBColor, 6> polygon_colors;
    std::array<big_int16_t, 5> random_sounds;
    std::array<SceneryDefinition, 61> scenery_definitions;
    std::map<Tag, std::vector<char>> tags;
    std::array<WeaponInterfaceDefinition, 10> weapon_interface_definitions;
};

void Fuxstate::diff(Fuxstate& other)
{
    pt::ptree tree;

    tree.add("<xmlcomment>", "Generated by fuxdiff");

    for (auto i = 0; i < control_panels.size(); ++i) {
        auto child = control_panels[i].diff(i, other.control_panels[i]);
        if (!child.empty()) {
            tree.add_child("marathon.control_panels.panel", child.get_child("panel"));
        }
    }
    
    for (auto i = 0; i < fade_definitions.size(); ++i) {
        auto child = fade_definitions[i].diff(i, other.fade_definitions[i]);
        if (!child.empty()) {
            tree.add_child("marathon.faders.fader", child.get_child("fader"));
        }
    }

    for (auto i = 0; i < infravision_colors.size(); ++i) {
        auto child = infravision_colors[i].diff(i, other.infravision_colors[i]);
        if (!child.empty()) {
            tree.add_child("marathon.infravision.color", child.get_child("color"));
        }
    }

    // overhead map colors
    for (auto i = 0; i < polygon_colors.size(); ++i) {
        auto color_tree = polygon_colors[i].diff(i, other.polygon_colors[i]);
        if (!color_tree.empty()) {
            tree.add_child("marathon.overhead_map.color", color_tree.get_child("color"));
        }
    }
    
    for (auto i = 0; i < line_definitions.size(); ++i) {
        auto color_tree = line_definitions[i].color.diff(i + 8, other.line_definitions[i].color);
        if (!color_tree.empty()) {
            tree.add_child("marathon.overhead_map.color", color_tree.get_child("color"));
        }
    }

    {
        auto color_tree = annotation_definition.color.diff(16, other.annotation_definition.color);
        if (!color_tree.empty()) {
            tree.add_child("marathon.overhead_map.color", color_tree.get_child("color"));
        }
    }

    {
        auto color_tree = map_name_color.diff(17, other.map_name_color);
        if (!color_tree.empty()) {
            tree.add_child("marathon.overhead_map.color", color_tree.get_child("color"));
        }
    }

    // overhead map lines
    for (auto i = 0 ; i < line_definitions.size(); ++i) {
        for (auto j = 0; j < line_definitions[i].pen_sizes.size(); ++j) {
            if (line_definitions[i].pen_sizes[j] != other.line_definitions[i].pen_sizes[j])
            {
                pt::ptree line_tree;
                line_tree.put("line.<xmlattr>.type", i);
                line_tree.put("line.<xmlattr>.scale", j);
                line_tree.put("line.<xmlattr>.width", other.line_definitions[i].pen_sizes[j]);
                tree.add_child("marathon.overhead_map.line", line_tree.get_child("line"));
            }
        }
    }

    // overhead map fonts
    for (auto i = 0; i < annotation_definition.sizes.size(); ++i) {
        if (annotation_definition.font != other.annotation_definition.font ||
            annotation_definition.face != other.annotation_definition.face ||
            annotation_definition.sizes[i] != other.annotation_definition.sizes[i]) {
            pt::ptree font_tree;
            font_tree.put("font.<xmlattr>.index", i);
            switch (other.annotation_definition.font) {
            case 4:
                font_tree.put("font.<xmlattr>.name", "Monaco");
                break;
            case 22:
                font_tree.put("font.<xmlattr>.name", "Courier");
                break;
            default:
                assert(false);
            }
            font_tree.put("font.<xmlattr>.size", other.annotation_definition.sizes[i]);
            font_tree.put("font.<xmlattr>.style", other.annotation_definition.face);
            tree.add_child("marathon.overhead_map.font", font_tree.get_child("font"));
        }
    }
    
    for (auto i = 0; i < damage_responses.size(); ++i) {
        auto child = damage_responses[i].diff(other.damage_responses[i], i);
        if (!child.empty()) {
            tree.add_child("marathon.player.damage", child.get_child("damage"));
        }
    }
    
    for (auto i = 0; i < media_definitions.size(); ++i) {
        auto child = media_definitions[i].diff(i, other.media_definitions[i]);
        if (!child.empty()) {
            tree.add_child("marathon.liquids.liquid", child.get_child("liquid"));
        }
    }

    for (auto i = 0; i < random_sounds.size(); ++i) {
        if (random_sounds[i] != other.random_sounds[i]) {
            pt::ptree random_tree;
            random_tree.put("random.<xmlattr>.index", i);
            random_tree.put("random.<xmlattr>.sound", other.random_sounds[i]);

            tree.add_child("marathon.sounds.random", random_tree.get_child("random"));
        }
    }

    for (auto i = 0; i < scenery_definitions.size(); ++i) {
        auto child = scenery_definitions[i].diff(i, other.scenery_definitions[i]);
        if (!child.empty()) {
            tree.add_child("marathon.scenery.object", child.get_child("object"));
        }
    }

    for (auto i = 0; i < weapon_interface_definitions.size(); ++i) {
        auto child = weapon_interface_definitions[i].diff(i, other.weapon_interface_definitions[i]);
        if (!child.empty()) {
            tree.add_child("marathon.interface.weapon", child.get_child("weapon"));
        }
    }

    std::ostringstream oss;
    pt::xml_writer_settings<std::string> settings(' ', 4);
    pt::write_xml(std::cout, tree, settings);

    auto physics_differ = false;
    
    for (auto kvp : tags) {
        if (other.tags[kvp.first] != kvp.second) {
            if (kvp.first == Tag{'E','f','f','x'} ||
                kvp.first == Tag{'I','t','e','m'} ||
                kvp.first == Tag{'M','o','n','s'} ||
                kvp.first == Tag{'P','r','o','j'} ||
                kvp.first == Tag{'W','e','p','1'})
            {
                physics_differ = true;
            } else if (kvp.first == Tag{'I','v','r','m'}) {
                std::cerr << "'Ivrm' differs, but Aleph One does not support 8-bit infravision MML\n";
            } else {
                std::cerr << kvp.first << " differs (" << other.tags[kvp.first].size() << ")\n";
            }
        }
    }

    if (physics_differ) {
        std::cerr << "Physics models differ\n";
    }
}

void Fuxstate::load(const char* filename)
{
    std::ifstream ifs(filename);
    load(ifs);
}

void Fuxstate::load(std::istream& s)
{
    while (s) {
        Header header;
        if (!s.read(reinterpret_cast<char*>(&header), sizeof(Header))) {
            break;
        }

        if (header.tag == Tag{'C','l','f','x'}) {
            assert(header.length == 768);
            s.read(reinterpret_cast<char*>(fade_definitions.data()), 768);
        } else if (header.tag == Tag{'D','a','m','g'}) {
            assert(header.length = 288);
            s.read(reinterpret_cast<char*>(damage_responses.data()), 288);
        } else if (header.tag == Tag{'I','v','c','l'}) {
            assert(header.length == 24);
            s.read(reinterpret_cast<char*>(infravision_colors.data()), 24);
        } else if (header.tag == Tag{'M','d','i','a'}) {
            assert(header.length == 260);
            s.read(reinterpret_cast<char*>(media_definitions.data()), 260);
        } else if (header.tag == Tag{'M','p','l','n'}) {
            assert(header.length == 42);
            s.read(reinterpret_cast<char*>(line_definitions.data()), 42);
        } else if (header.tag == Tag{'M','p','n','c'}) {
            assert(header.length == 6);
            s.read(reinterpret_cast<char*>(&map_name_color), 6);
        } else if (header.tag == Tag{'M','p','p','l'}) {
            assert(header.length == 36);
            s.read(reinterpret_cast<char*>(polygon_colors.data()), 36);
        } else if (header.tag == Tag{'M','p','t','x'}) {
            assert(header.length == 18);
            s.read(reinterpret_cast<char*>(&annotation_definition), 18);
        } else if (header.tag == Tag{'P','a','n','l'}) {
            assert(header.length == 1188);
            s.read(reinterpret_cast<char*>(control_panels.data()), 1188);
        } else if (header.tag == Tag{'R','a','n','d'}) {
            assert(header.length == 10);
            s.read(reinterpret_cast<char*>(random_sounds.data()), 10);
        } else if (header.tag == Tag{'S','c','n','r'}) {
            assert(header.length == 732);
            s.read(reinterpret_cast<char*>(scenery_definitions.data()), 732);
        } else if (header.tag == Tag{'T','y','p','e'}) {
            assert(header.length == 28);
            // there's no meaningful way to translate this to MML
            s.seekg(28, s.cur);
        } else if (header.tag == Tag{'W','e','p','2'}) {
            assert(header.length == 580);
            s.read(reinterpret_cast<char*>(weapon_interface_definitions.data()), 580);
        } else {
            auto& v = tags[header.tag];
            v.resize(header.length);
            
            s.read(v.data(), v.size());
        }
    }
}

int main(int argv, char* argc[])
{
    if (argv != 3) {
        std::cerr << "Usage: fuxdiff <base> <modified>\n";
        return -1;
    }

    Fuxstate base;
    base.load(argc[1]);

    Fuxstate mod;
    mod.load(argc[2]);

    base.diff(mod);
}

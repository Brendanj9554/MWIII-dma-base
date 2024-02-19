#pragma once
#include "../driver/dma.h"

namespace sdk {
    uintptr_t module_base = 0;
    uintptr_t peb = 0;
}

struct NameEntry
{
    uint32_t index;
    char name[36];
    uint8_t pad[92];
    int32_t health;
    uint8_t pad2[70];
};

enum bone : int
{
    bone_pos_helmet = 8,

    bone_pos_head = 7,
    bone_pos_neck = 6,
    bone_pos_chest = 5,
    bone_pos_mid = 4,
    bone_pos_tummy = 3,
    bone_pos_pelvis = 2,

    bone_pos_right_foot_1 = 21,
    bone_pos_right_foot_2 = 22,
    bone_pos_right_foot_3 = 23,
    bone_pos_right_foot_4 = 24,

    bone_pos_left_foot_1 = 17,
    bone_pos_left_foot_2 = 18,
    bone_pos_left_foot_3 = 19,
    bone_pos_left_foot_4 = 20,

    bone_pos_left_hand_1 = 13,
    bone_pos_left_hand_2 = 14,
    bone_pos_left_hand_3 = 15,
    bone_pos_left_hand_4 = 16,

    bone_pos_right_hand_1 = 9,
    bone_pos_right_hand_2 = 10,
    bone_pos_right_hand_3 = 11,
    bone_pos_right_hand_4 = 12
};

enum offset {
    refdef = 0x1069A1C0,
    game_mode = 0xD1EDF60,
    local_index = 0x522F8,
    local_index_pos = 0x02F0,
    player_size = 0x13DA8,
    name_arrary = 0x1069A188,
    name_arrary_pos = 0x2C80,
    name_size = 0xC0,
    player_valid = 0x18ED,
    player_team = 0x1298,
    player_pos = 0x1B10,
    index_struct_size = 0x188,
    camera_base = 0x10F0F020,
    camera_position = 0x204,
    bone_base_position = 0x90F48,
};

class Pointers {
public:
    uintptr_t
        client_info,
        client_info_base,
        bone_base,
        bone_index,
        currentvisoffset,
        last_visible_offset,
        player_controller,
        root_component,
        skeletal_mesh,
        persistent_level,
        player_array,
        current_weapon,
        current_vehicle;
    int
        player_array_size,
        team_index;
    fvector
        relative_location;
}; static Pointers* pointer = new Pointers();

struct camera {
    fvector location;
    fvector rotation;
    float field_of_view;
    char Useless[0x18];
};
class player {
public:
    player(uintptr_t address) {
        this->address = address;
    }

    uintptr_t address{};

    int id{};

    bool is_player_valid();

    bool is_dead();


    int team_id();

    fvector get_position();
    uint32_t get_index();
    NameEntry get_name_entry(uint32_t index);
    uintptr_t bone_pointer(uint64_t bone_base, uint64_t bone_index);
};
struct ref_def_view {
    fvector2d tan_half_fov;
    char pad[0xC];
    fvector axis[3];
};

struct ref_def_t {
    int x;
    int y;
    int width;
    int height;
    ref_def_view view;
};
class refdef_class {

public:
    struct key {
        int ref0;
        int ref1;
        int ref2;
    };

    auto retrieve_ref_def() -> uintptr_t {
        key encrypted = DMA::Read<key>(sdk::module_base + offset::refdef, sizeof(key));

        DWORD lowerref = encrypted.ref0 ^ (encrypted.ref2 ^ (uint64_t)(sdk::module_base + offset::refdef)) * ((encrypted.ref2 ^ (uint64_t)(sdk::module_base + offset::refdef)) + 2);
        DWORD upperref = encrypted.ref1 ^ (encrypted.ref2 ^ (uint64_t)(sdk::module_base + offset::refdef + 0x4)) * ((encrypted.ref2 ^ (uint64_t)(sdk::module_base + offset::refdef + 0x4)) + 2);

        return (uint64_t)upperref << 32 | lowerref;
    }
    ref_def_t ref_def_nn;
};
static refdef_class* decrypt_refdef = new refdef_class();
namespace utilities {


    class c_utilities {
    public:


        auto InScreen(fvector2d screen_position) -> bool {

            if (screen_position.x > 0 && screen_position.x < globals->width && screen_position.y > 0 && screen_position.y < globals->height)
                return true;
            else
                return false;

        }

        auto IsShootable(fvector lur, fvector wl) -> bool {

            if (lur.x >= wl.x - 20 && lur.x <= wl.x + 20 && lur.y >= wl.y - 20 && lur.y <= wl.y + 20 && lur.z >= wl.z - 30 && lur.z <= wl.z + 30)
                return true;
            else
                return false;

        }

        auto is_user_in_game() -> bool {
            auto ingame = DMA::Read<int>(sdk::module_base + offset::game_mode, sizeof(int)) > 1;
            return ingame;
        }
        auto retrieve_name_list() -> uint64_t {
            auto ptr = DMA::Read<uint64_t>(sdk::module_base + offset::name_arrary, sizeof(uint64_t));
            return ptr + offset::name_arrary_pos;
        }
        auto player_count() -> int {
            auto playercount = DMA::Read<int>(sdk::module_base + offset::game_mode, sizeof(int));
            return playercount;
        }
        auto local_player_index() -> int {
            auto index = DMA::Read<uintptr_t>(pointer->client_info + offset::local_index, sizeof(uintptr_t));
            auto read_return = DMA::Read<int>(index + offset::local_index_pos, sizeof(int));
            return read_return;
        }

        auto retrieve_camera_posistion() -> fvector {
            auto player_camera = DMA::Read<uintptr_t>(sdk::module_base + offset::camera_base, sizeof(uintptr_t));
            if (!player_camera)
                return {}; // Return an empty fvector if player_camera is null

            auto received = DMA::Read<fvector>(player_camera + offset::camera_position, sizeof(fvector));
            return received;
        }
        auto get_angles() -> fvector2d {
            auto camera_angle = DMA::Read<uintptr_t>(sdk::module_base + offset::camera_base, sizeof(uintptr_t));
            if (!camera_angle)
                return {}; // Return an empty fvector2d if camera_angle is null

            auto received = DMA::Read<fvector2d>(camera_angle + offset::camera_position + 0xC, sizeof(fvector2d));
            return received;
        }

        auto retrieve_bone_position(const uintptr_t pointer, const fvector& bone_position, const int Bone) -> fvector
        {
            fvector position = DMA::Read<fvector>(pointer + ((uint64_t)Bone * 0x20) + 0x10, sizeof(fvector));
            position.x += bone_position.x;
            position.y += bone_position.y;
            position.z += bone_position.z;
            return position;
        }

        auto retrieve_bone_position_vec(const uintptr_t Client_Information) -> fvector {
            auto information = DMA::Read<fvector>(Client_Information + offset::bone_base_position, sizeof(fvector));
            return information;
        }
        auto world_to_screen(fvector world_location, fvector2d& out, fvector camera_pos, int screen_width, int screen_height, fvector2d fov, fvector matricies[3]) -> bool
        {
            auto local = world_location - camera_pos;
            auto trans = fvector{
                local.Dot(matricies[1]),
                local.Dot(matricies[2]),
                local.Dot(matricies[0])
            };

            if (trans.z < 0.01f) {
                return false;
            }

            out.x = ((float)screen_width / 2.0) * (1.0 - (trans.x / fov.x / trans.z));
            out.y = ((float)screen_height / 2.0) * (1.0 - (trans.y / fov.y / trans.z));

            if (out.x < 1 || out.y < 1 || (out.x > decrypt_refdef->ref_def_nn.width) || (out.y > decrypt_refdef->ref_def_nn.height)) {
                return false;
            }

            return true;
        }

        auto w2s(fvector world_position, fvector2d& screen_position) -> bool
        {
            return world_to_screen(world_position, screen_position, retrieve_camera_posistion(), decrypt_refdef->ref_def_nn.width, decrypt_refdef->ref_def_nn.height, decrypt_refdef->ref_def_nn.view.tan_half_fov, decrypt_refdef->ref_def_nn.view.axis);
        }

        auto units_to_m(float units) -> float {
            return units * 0.0254;
        }

        auto ConvertDistanceToString(float dist) -> std::string
        {
            std::stringstream strs;
            strs << dist;
            std::string temp_str = strs.str();
            const char* text = (const char*)temp_str.c_str();
            return (std::string)text + "m";
        }

        auto cursor_to(float x, float y) -> void {
            fvector center(globals->width / 2, globals->height / 2, 0);
            fvector target(0, 0, 0);

            if (x != 0)
            {
                if (x > center.x)
                {
                    target.x = -(center.x - x);
                    target.x /= (globals->smooth + 3);
                    if (target.x + center.x > center.x * 2)
                        target.x = 0;
                }

                if (x < center.x)
                {
                    target.x = x - center.x;
                    target.x /= (globals->smooth + 3);
                    if (target.x + center.x < 0)
                        target.x = 0;
                }
            }
            if (y != 0)
            {
                if (y > center.y)
                {
                    target.y = -(center.y - y);
                    target.y /= (globals->smooth + 3);
                    if (target.y + center.y > center.y * 2)
                        target.y = 0;
                }

                if (y < center.y)
                {
                    target.y = y - center.y;
                    target.y /= (globals->smooth + 3);
                    if (target.y + center.y < 0)
                        target.y = 0;
                }
            }

            auto offset_x = static_cast<float>(rand() % 5) / 10.0f - 1.5f;
            auto offset_y = static_cast<float>(rand() % 5) / 10.0f - 1.5f;

            Inject->set_cursor_position(target.x + offset_x, target.y + offset_y);
        }

        static float powf_(float _X, float _Y) {
            return (_mm_cvtss_f32(_mm_pow_ps(_mm_set_ss(_X), _mm_set_ss(_Y))));
        }
        static float sqrtf_(float _X) {
            return (_mm_cvtss_f32(_mm_sqrt_ps(_mm_set_ss(_X))));
        }
        static double GetCrossDistance(double x1, double y1, double x2, double y2) {
            return sqrtf(powf((x2 - x1), 2) + powf_((y2 - y1), 2));
        }

    };
} static utilities::c_utilities* Utilities = new utilities::c_utilities();


auto player::get_index() -> uint32_t
{
    auto index = (this->address - sdk::module_base) / offset::player_size;
    return index;
}
auto player::get_name_entry(uint32_t p_index) -> NameEntry {
    auto name = DMA::Read<NameEntry>(Utilities->retrieve_name_list() + (p_index * offset::name_size), sizeof(NameEntry));
    return name;
}
auto player::is_player_valid() -> bool {
    auto response = DMA::Read<bool>(this->address + offset::player_valid, sizeof(bool));
    return response;
}
auto player::team_id() -> int {
    auto team = DMA::Read<int>(this->address + offset::player_team, sizeof(int));
    return team;
}
auto player::get_position() -> fvector {
    auto local_position_ptr = DMA::Read<uintptr_t>(this->address + offset::player_pos, sizeof(uintptr_t));
    if (!local_position_ptr)
        return {}; 

    auto final_pos = DMA::Read<fvector>(local_position_ptr + 0x48, sizeof(fvector));
    return final_pos;
}
auto player::bone_pointer(uint64_t base, uint64_t index) -> uintptr_t {
    auto bone_address = DMA::Read<uintptr_t>(base + (index * offset::index_struct_size) + 0xD8, sizeof(uintptr_t));
    return bone_address;
}
class decryption
{
public:

    auto Client_Information() -> uint64_t {
        uint64_t mb = sdk::module_base;
        uint64_t peb = sdk::peb;

        uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;
        rbx = DMA::Read<uintptr_t>(sdk::module_base + 0x104C2408, sizeof(uintptr_t));
        if (!rbx)
            return rbx;
        rcx = ~sdk::peb;              //mov rcx, gs:[rax]
        rdx = sdk::module_base;              //lea rdx, [0xFFFFFFFFFE31F16B]
        rax = 0;                //and rax, 0xFFFFFFFFC0000000
        rbx -= rdx;             //sub rbx, rdx
        rax = _rotl64(rax, 0x10);               //rol rax, 0x10
        rax ^= DMA::Read<uintptr_t>(sdk::module_base + 0x70F00C3, sizeof(uintptr_t));              //xor rax, [0x000000000540F21A]
        rax = ~rax;             //not rax
        rbx *= DMA::Read<uintptr_t>(rax + 0x13, sizeof(uintptr_t));            //imul rbx, [rax+0x13]
        rax = 0xBD20874E271585EB;               //mov rax, 0xBD20874E271585EB
        rbx *= rax;             //imul rbx, rax
        rax = 0x35860EA22C1F2550;               //mov rax, 0x35860EA22C1F2550
        rbx -= rax;             //sub rbx, rax
        rax = sdk::module_base + 0x9F80;             //lea rax, [0xFFFFFFFFFE3290AD]
        rax += rcx;             //add rax, rcx
        rbx ^= rax;             //xor rbx, rax
        rax = rbx;              //mov rax, rbx
        rax >>= 0x28;           //shr rax, 0x28
        rbx ^= rax;             //xor rbx, rax
        return rbx;

    }

    auto Client_Base(uintptr_t client_info) -> uintptr_t {
        uint64_t mb = sdk::module_base;
        uint64_t peb = sdk::peb;

        uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;
        rdx = DMA::Read<uintptr_t>(client_info + 0x190418, sizeof(uintptr_t));
        if (!rdx)
            return rdx;

        r8 = ~sdk::peb;               //mov r8, gs:[rax]
        rax = rdx;              //mov rax, rdx
        rax >>= 0x12;           //shr rax, 0x12
        rdx ^= rax;             //xor rdx, rax
        rax = rdx;              //mov rax, rdx
        rax >>= 0x24;           //shr rax, 0x24
        rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
        rdx ^= rax;             //xor rdx, rax
        rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
        rcx ^= DMA::Read<uintptr_t>(sdk::module_base + 0x70F00FE, sizeof(uintptr_t));             //xor rcx, [0x000000000540F554]
        rax = 0x7F668ABCF33A470D;               //mov rax, 0x7F668ABCF33A470D
        rcx = _byteswap_uint64(rcx);            //bswap rcx
        rdx *= DMA::Read<uintptr_t>(rcx + 0x13, sizeof(uintptr_t));            //imul rdx, [rcx+0x13]
        rdx *= rax;             //imul rdx, rax
        rax = sdk::module_base + 0x3D23;             //lea rax, [0xFFFFFFFFFE32315C]
        rdx ^= r8;              //xor rdx, r8
        rdx ^= rax;             //xor rdx, rax
        rax = rdx;              //mov rax, rdx
        rax >>= 0x21;           //shr rax, 0x21
        rdx ^= rax;             //xor rdx, rax
        rax = rdx;              //mov rax, rdx
        rax >>= 0x16;           //shr rax, 0x16
        rdx ^= rax;             //xor rdx, rax
        rax = rdx;              //mov rax, rdx
        rax >>= 0x2C;           //shr rax, 0x2C
        rdx ^= rax;             //xor rdx, rax
        return rdx;

    }

    auto bone_base() -> uintptr_t {
        uint64_t mb = sdk::module_base;
        uint64_t peb = sdk::peb;

        uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;
        rcx = DMA::Read<uintptr_t>(sdk::module_base + 0xAF36518, sizeof(uintptr_t));
        if (!rcx)
            return rcx;

        rdx = sdk::peb;              //mov rdx, gs:[rax]
        rax = 0;                //and rax, 0xFFFFFFFFC0000000
        rax = _rotl64(rax, 0x10);               //rol rax, 0x10
        rax ^= DMA::Read<uintptr_t>(sdk::module_base + 0x70F01EE, sizeof(uintptr_t));              //xor rax, [0x0000000002EE305E]
        rax = ~rax;             //not rax
        rcx *= DMA::Read<uintptr_t>(rax + 0x13, sizeof(uintptr_t));            //imul rcx, [rax+0x13]
        rax = 0xE500FD3290E1F913;               //mov rax, 0xE500FD3290E1F913
        rcx *= rax;             //imul rcx, rax
        rax = 0x94AEADE7D7695FD6;               //mov rax, 0x94AEADE7D7695FD6
        rcx += rdx;             //add rcx, rdx
        rdx = sdk::module_base;              //lea rdx, [0xFFFFFFFFFBDF2E46]
        rcx += rax;             //add rcx, rax
        rcx += rdx;             //add rcx, rdx
        rax = rcx;              //mov rax, rcx
        rax >>= 0x9;            //shr rax, 0x09
        rcx ^= rax;             //xor rcx, rax
        rax = rcx;              //mov rax, rcx
        rax >>= 0x12;           //shr rax, 0x12
        rcx ^= rax;             //xor rcx, rax
        rax = rcx;              //mov rax, rcx
        rax >>= 0x24;           //shr rax, 0x24
        rcx ^= rax;             //xor rcx, rax
        rax = rcx;              //mov rax, rcx
        rax >>= 0xD;            //shr rax, 0x0D
        rcx ^= rax;             //xor rcx, rax
        rax = rcx;              //mov rax, rcx
        rax >>= 0x1A;           //shr rax, 0x1A
        rcx ^= rax;             //xor rcx, rax
        rax = rcx;              //mov rax, rcx
        rax >>= 0x34;           //shr rax, 0x34
        rcx ^= rax;             //xor rcx, rax
        return rcx;


    }

    auto bone_index(uint32_t index) -> uint64_t {
        const uint64_t mb = sdk::module_base;

        uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;
        rsi = index;
        rcx = rsi * 0x13C8;
        rax = 0xC4663A35CC80B1FD;               //mov rax, 0xC4663A35CC80B1FD
        r11 = DMA::BaseAddress;              //lea r11, [0xFFFFFFFFFE3157D3]
        rax = _umul128(rax, rcx, (uintptr_t*)&rdx);             //mul rcx
        r10 = 0xAA07B899C1FB6F0B;               //mov r10, 0xAA07B899C1FB6F0B
        rdx >>= 0xC;            //shr rdx, 0x0C
        rax = rdx * 0x14DB;             //imul rax, rdx, 0x14DB
        rcx -= rax;             //sub rcx, rax
        rax = 0x4F98277C7B96C15;                //mov rax, 0x4F98277C7B96C15
        r8 = rcx * 0x14DB;              //imul r8, rcx, 0x14DB
        rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
        rdx >>= 0x7;            //shr rdx, 0x07
        rax = rdx * 0x19BB;             //imul rax, rdx, 0x19BB
        r8 -= rax;              //sub r8, rax
        rax = 0xB244C4069D8D463F;               //mov rax, 0xB244C4069D8D463F
        rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
        rcx = r8;               //mov rcx, r8
        r8 &= 0x3;             //and r8d, 0x03
        rdx >>= 0xB;            //shr rdx, 0x0B
        rax = rdx * 0xB7D;              //imul rax, rdx, 0xB7D
        rcx -= rax;             //sub rcx, rax
        rax = r8 + rcx * 4;             //lea rax, [r8+rcx*4]
        rax = DMA::Read<uint16_t>(r11 + rax * 2 + 0x71670B0, sizeof(rax));                //movzx eax, word ptr [r11+rax*2+0x71670B0]
        r8 = rax * 0x13C8;              //imul r8, rax, 0x13C8
        rax = r10;              //mov rax, r10
        rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
        rax = r10;              //mov rax, r10
        rdx >>= 0xC;            //shr rdx, 0x0C
        rcx = rdx * 0x1817;             //imul rcx, rdx, 0x1817
        r8 -= rcx;              //sub r8, rcx
        r9 = r8 * 0x261A;               //imul r9, r8, 0x261A
        rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
        rdx >>= 0xC;            //shr rdx, 0x0C
        rax = rdx * 0x1817;             //imul rax, rdx, 0x1817
        r9 -= rax;              //sub r9, rax
        rax = 0x8FB823EE08FB823F;               //mov rax, 0x8FB823EE08FB823F
        rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
        rax = 0xBD69104707661AA3;               //mov rax, 0xBD69104707661AA3
        rdx >>= 0x5;            //shr rdx, 0x05
        rcx = rdx * 0x39;               //imul rcx, rdx, 0x39
        rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
        rdx >>= 0x7;            //shr rdx, 0x07
        rcx += rdx;             //add rcx, rdx
        rax = rcx * 0x15A;              //imul rax, rcx, 0x15A
        rcx = r9 * 0x15C;               //imul rcx, r9, 0x15C
        rcx -= rax;             //sub rcx, rax
        r8 = DMA::Read<uint16_t>(rcx + r11 * 1 + 0x716CCA0, sizeof(r8));                //movsx r8d, word ptr [rcx+r11*1+0x716CCA0]
        return r8;


    }
};
static decryption* decrypt = new decryption();
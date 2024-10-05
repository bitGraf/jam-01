#include "Sprite.h"

#include "logging.h"

#include <SDL.h>
#include <SDL_image.h>

// json parser
#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

/////////////////////////////////////////////////////////////////////
// Sprite_Sequence
/////////////////////////////////////////////////////////////////////

void Sprite_Sequence::Update(real32 dt) {
    sequence_time += dt;

    real32 frame_time = 1.0 / frame_rate;
    real32 length = frame_time * num_frames;

    if (sequence_time > length) {
        // if looping, reset to zero.
        // otherwise, do nothing!
        if (looping) {
            while (sequence_time > length)
                sequence_time -= length;
        } else {
            sequence_time = length;
        }
    }

    current_frame = (int32)(sequence_time / frame_time);
    current_frame = current_frame >= num_frames ? num_frames-1 : current_frame;
}

const Sprite_Frame& Sprite_Sequence::Current_Frame() const {
    return this->frames[current_frame];
}



/////////////////////////////////////////////////////////////////////
// Sprite
/////////////////////////////////////////////////////////////////////

Sprite::Sprite() {

}

bool Sprite::Load_Sprite_Sheet(SDL_Renderer* renderer, const char* filename, int32 sprite_w, int32 sprite_h, real32 anchor_x, real32 anchor_y) {
    // Load image into texture
    int sheet_w = 0;
    int sheet_h = 0;
    {
        SDL_Surface* image = IMG_Load(filename);
        if (!image) {
            log_fatal("Error loading image [%s]: %s", filename, SDL_GetError());
            return false;
        }

        sheet_w = image->w;
        sheet_h = image->h;

        texture = SDL_CreateTextureFromSurface(renderer, image);
        SDL_FreeSurface(image);
        image = nullptr;
    }
    if (!texture) {
        log_fatal("Error creating texture from surface: %s", SDL_GetError());
        return false;
    }

    // Slice into frames
    int num_x = (sheet_w / sprite_w);
    int num_y = (sheet_h / sprite_h);
    if ((num_x * sprite_w != sheet_w) || (num_y * sprite_h != sheet_h)) {
        log_warn("Sheet is %dx%d, does not divide nicely into %dx%d blocks.", sheet_w, sheet_h, sprite_w, sprite_h);
    }

    this->sequences.clear();
    this->sequences.reserve(num_y);
    for (int y_idx = 0; y_idx < num_y; y_idx++) {
        int y = (y_idx*sprite_h);
        
        Sprite_Sequence sequence;
        sequence.frames.reserve(num_x);
        
        for (int x_idx = 0; x_idx < num_x; x_idx++) {
            int x = (x_idx*sprite_w);

            Sprite_Frame frame;

            frame.x_index = x_idx;
            frame.y_index = y_idx;

            frame.x = x;
            frame.y = y;
            frame.w = sprite_w;
            frame.h = sprite_h;

            sequence.frames.push_back(frame);
        }

        sequence.current_frame = 0;
        sequence.looping = true;
        sequence.sequence_time = 0.0;
        sequence.frame_rate = 1.0;
        sequence.num_frames = num_x; // max number

        this->sequences.push_back(sequence);
    }
    this->current_sequence = 0;

    this->size = laml::Vec2(sprite_w, sprite_h);
    this->anchor = laml::Vec2(anchor_x, anchor_y);

    return true;
}

bool Sprite::Load_Sprite_Sheet_From_Meta(SDL_Renderer* renderer, const char* filename) {
    std::ifstream json_file(filename);
    try {
        json data = json::parse(json_file);

        // spritesheet required
        if (data.find("Sprite_Sheet") == data.end()) {
            log_error("Sprite meta-file [%s] missing spritesheet!", filename);
            return false;
        }
        std::string spritesheet = data["Sprite_Sheet"].get<std::string>();
        
        // if not find width, use deafault
        real32 sprite_width = 16;
        if (data.find("Sprite_Width") != data.end()) {
            sprite_width = data["Sprite_Width"].get<real32>();
        }

        // if not find width, use same as width
        real32 sprite_height = sprite_width;
        if (data.find("Sprite_Height") != data.end()) {
            sprite_height = data["Sprite_Height"].get<real32>();
        }

        // get anchor
        real32 anchor_x = 0.0;
        if (data.find("Anchor_X") != data.end()) {
            anchor_x = data["Anchor_X"].get<real32>();
        }
        real32 anchor_y = 0.0;
        if (data.find("Anchor_Y") != data.end()) {
            anchor_y = data["Anchor_Y"].get<real32>();
        }

        if (!Load_Sprite_Sheet(renderer, spritesheet.c_str(), sprite_width, sprite_height, anchor_x, anchor_y)) {
            return false;
        }

        // get sequences
        if (data.find("Sequences") != data.end()) {
            json sequence_list = data["Sequences"];
            if (!sequence_list.is_array()) {
                log_error("json 'Sequences' is not an array");
                return false;
            }

            if (sequence_list.size() > this->sequences.size())  {
                log_warn("Spritesheet implies %d sequences, json implies %d", sequence_list.size(), this->sequences.size());
            }

            int sequence_idx = 0;
            for (const json& sequence : sequence_list) {
                if (sequence_idx >= this->sequences.size()) {
                    log_error("Ignoring Json sequence #%d", sequence_idx+1);
                    continue;
                }
                this->sequences[sequence_idx].name       = sequence["name"];
                int32 num_frames = sequence["num_frames"].get<int32>();
                if (num_frames > 0) {
                    this->sequences[sequence_idx].num_frames = num_frames;
                }
                this->sequences[sequence_idx].frame_rate = sequence["frame_rate"];
                this->sequences[sequence_idx].looping    = sequence["looping"];

                sequence_idx++;
            }
        }

    } catch (json::parse_error e) {
        log_error("Json parse exception: [%s]", e.what());
        return false;
    } catch (json::type_error e) {
        log_error("Json type exception: [%s]", e.what());
        return false;
    } catch (json::other_error e) {
        log_error("Json other exception: [%s]", e.what());
        return false;
    }

    return true;
}

void Sprite::Destroy() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}

void Sprite::Update(real32 dt) {
    Sprite_Sequence& sequence = this->sequences[current_sequence];
    sequence.Update(dt);
}

const Sprite_Frame& Sprite::Current_Frame() const {
    return this->sequences[current_sequence].Current_Frame();
}

void Sprite::Set_Sequence(uint8 sequence_idx, uint8 flags) {
    uint8 num_sequences = this->sequences.size();
    if (sequence_idx >= num_sequences) {
        log_error("Tried to set sequence to %d, but only %d exist.", sequence_idx, num_sequences);
        return;
    }

    this->current_sequence = sequence_idx;

    // operate on flags
    // ...
    Sprite_Sequence& sequence = this->sequences[current_sequence];
    sequence.sequence_time = 0.0;
}
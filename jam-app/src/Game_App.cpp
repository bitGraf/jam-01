#include "game_app.h"

// json parser
#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

#include "logging.h"
#include "Render_Utils.h"

// Global vars :)
Game_App g_game;

TTF_Font* g_tiny_font;
TTF_Font* g_small_font;
TTF_Font* g_medium_font;
TTF_Font* g_large_font;
TTF_Font* g_huge_font;

int32 g_font_size_tiny = 12;
int32 g_font_size_small = 24;
int32 g_font_size_medium = 36;
int32 g_font_size_large = 48;
int32 g_font_size_huge = 72;

int32 g_window_width = 800;
int32 g_window_height = 600;
uint32 g_window_pixel_format = 0;

// local vars
static const real32 deadzone = 0.5;
static bool draw_input_debug = false;
static std::string window_name = "game-app";

Game_App::Game_App() {}
Game_App::~Game_App() {}

bool Game_App::Init() {
    // read init.json
    if (!ReadConfig()) {
        log_fatal("Error reading init.json!");

		// End the program
		return false;
    }

    // Initialize SDL. SDL_Init will return -1 if it fails.
	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
        log_fatal("Error initializing SDL: %s", SDL_GetError());

		// End the program
		return false;
	} 

    // Initialize Extension Libraries
    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) == 0) {
        log_fatal("Error initializing SDL_Image: %s", IMG_GetError());
    
		// End the program
		return false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0) {
        log_fatal("Error initializing SDL_Mixer: %s", Mix_GetError());
        return false;
    }
    if (TTF_Init() < 0) {
        log_fatal("Error initializing SDL_TTF: %s", TTF_GetError());
        return false;
    }

    // Create our window
	window = SDL_CreateWindow( window_name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, g_window_width, g_window_height, SDL_WINDOW_SHOWN );
    g_window_pixel_format = SDL_GetWindowPixelFormat(window);

	// Make sure creating the window succeeded
	if ( !window ) {
        log_fatal("Error creating window: %s", SDL_GetError());

		// End the program
		return false;
	}

    // create renderer
    //renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        log_fatal("Error creating renderer: %s", SDL_GetError());

		// End the program
		return false;
    }

    gamepad.Set_Deadzones(deadzone, deadzone, deadzone, deadzone);
    Update_Volume(options.master_volume, options.music_volume);

    log_info("SDL up and running!");

    return true;
}

bool Game_App::Shutdown() {
    WriteConfig();

    // clean up Game_State stack
    while (game_state.size() > 0) {
        Game_State* state = game_state.top();
        delete state;
        state = nullptr;
        game_state.pop();
    }
    
    // Destroy sounds
    Mix_FreeChunk(sound);
    Mix_FreeMusic(music);

    // Destroy fonts
    TTF_CloseFont(g_small_font);
    TTF_CloseFont(g_medium_font);
    TTF_CloseFont(g_large_font);

	// Destroy the window. This will also destroy the surface
    SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	// Quit SDL and extensions
    IMG_Quit();
    Mix_Quit();
    TTF_Quit();
	SDL_Quit();

    return true;
}

bool Game_App::Load_Assets() {
    // load cursor
    {
        SDL_Surface* image = IMG_Load("data/cursor_final.png");
        if (!image) {
            log_fatal("Error loading image [data/cursor_final.png]: %s", SDL_GetError());
            return false;
        }

        cursor = SDL_CreateTextureFromSurface(renderer, image);
        SDL_FreeSurface(image);
        image = NULL;
    }
    if (!cursor) {
        log_fatal("Error creating cursor from surface: %s", SDL_GetError());
        return false;
    }

    // Load sounds
    sound = Mix_LoadWAV("data/Guard_death1.wav");
    music = Mix_LoadMUS("data/music.mp3");
    if (!music || !sound) {
        log_fatal("Failed to load music or sound: %s", Mix_GetError());
        return false;
    }

    // Load fonts
    g_tiny_font   = TTF_OpenFont("data/Silkscreen/slkscr.ttf", g_font_size_tiny);
    g_small_font  = TTF_OpenFont("data/Silkscreen/slkscr.ttf", g_font_size_small);
    g_medium_font = TTF_OpenFont("data/Silkscreen/slkscr.ttf", g_font_size_medium);
    g_large_font  = TTF_OpenFont("data/Silkscreen/slkscr.ttf", g_font_size_large);
    g_huge_font   = TTF_OpenFont("data/Silkscreen/slkscr.ttf", g_font_size_huge);

    log_info("All assets loaded!");

    return true;
}

bool Game_App::Run() {
    if (!Init()) {
        log_fatal("Game_App::Run() Failed to initialize");
        return false;
    }
    if (!Load_Assets()) {
        log_fatal("Game_App::Run() Failed to load assets");
        return false;
    }

    // hide cursor
    SDL_ShowCursor(SDL_DISABLE);

    // Start game in a menu state
    Game_State* menu = new Menu_State();
    this->game_state.push(menu);
    World_State* level1 = new World_State("data/levels/level_1.json");
    this->Push_New_State(level1);

    ////////////////////////////////////////////////////////////////////////////
    // Game Event Loop
    ////////////////////////////////////////////////////////////////////////////
    SDL_Event ev;
    bool running = true;
    size_t frame_number = 0;

    // timing
    const double target_framerate = 100.0;
    double target_frame_time = 1.0f / ((real32)target_framerate);
    double last_frame_time = target_frame_time;
    double lock_framerate = true;
    uint64 LastCounter = SDL_GetPerformanceCounter();
    double fps = 0.0;
    double fps_avg = 10.0;

    // joystick
    SDL_Joystick* joystick = NULL;

    SDL_Color font_color = {0, 0, 0};

    log_info("Starting game event loop.");
    Mix_PlayMusic(music, -1);
    while (running) {
        frame_number++;
        log_frame_start(frame_number);

        // blank screen
        SDL_SetRenderTarget(renderer, NULL);
        SDL_SetRenderDrawColor(renderer, 255, 90, 120, 255);
        SDL_RenderClear(renderer);

        joystick = Update_Joysticks();

        mouse.Update(last_frame_time);
        while (SDL_PollEvent(&ev) != 0) {
            // respond to events
            switch (ev.type) {
                case SDL_QUIT:
                    running = false;
                    log_debug("Event: SDL_QUIT");
                    break;

                // Gamepad input events
                case SDL_JOYBUTTONDOWN:
                case SDL_JOYBUTTONUP: {
                    gamepad.Button_Event(ev.jbutton.button, ev.jbutton.state == SDL_PRESSED);
                    Action_Event action = Action_Event_From_Button_Event(ev.jbutton.button, ev.jbutton.state == SDL_PRESSED);
                    this->On_Action_Event(action);
                    } break;
                case SDL_JOYHATMOTION: {
                    gamepad.Hat_Event(ev.jhat.value);

                    this->On_Action_Event(Action_Event_From_Hat_Event(ev.jhat.value & SDL_HAT_UP));
                    this->On_Action_Event(Action_Event_From_Hat_Event(ev.jhat.value & SDL_HAT_DOWN));
                    this->On_Action_Event(Action_Event_From_Hat_Event(ev.jhat.value & SDL_HAT_LEFT));
                    this->On_Action_Event(Action_Event_From_Hat_Event(ev.jhat.value & SDL_HAT_RIGHT));
                    } break;

                // Keyboard events
                case SDL_KEYUP:
                case SDL_KEYDOWN: {
                    Action_Event action = Action_Event_From_Key_Event(ev.key.keysym.sym, ev.key.state == SDL_PRESSED);
                    this->On_Action_Event(action);
                    On_Key_Event(ev.key.keysym.sym, ev.key.state == SDL_PRESSED);
                    } break;

                // Get mouse position
                case SDL_MOUSEMOTION:
                    mouse.Update_Motion(ev.motion.x, ev.motion.y, ev.motion.xrel, ev.motion.yrel);
                    break;
                
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                    mouse.Button_Event(ev.button.button, ev.button.state==SDL_PRESSED);
                    break;
                
                case SDL_MOUSEWHEEL:
                    mouse.Wheel_Event(ev.wheel.y, ev.wheel.preciseY);
                    break;
            }
        }

        // get axes
        //   |
        // --+-> x
        //   v
        //   y
        gamepad.Update_Axis(Joystick_Axes::LEFT_STICK_X,  SDL_JoystickGetAxis(joystick, 0));
        gamepad.Update_Axis(Joystick_Axes::LEFT_STICK_Y,  SDL_JoystickGetAxis(joystick, 1), true);
        gamepad.Update_Axis(Joystick_Axes::RIGHT_STICK_X, SDL_JoystickGetAxis(joystick, 2));
        gamepad.Update_Axis(Joystick_Axes::RIGHT_STICK_Y, SDL_JoystickGetAxis(joystick, 3), true);
        gamepad.Update_Axis(Joystick_Axes::LEFT_TRIGGER,  SDL_JoystickGetAxis(joystick, 4));
        gamepad.Update_Axis(Joystick_Axes::RIGHT_TRIGGER, SDL_JoystickGetAxis(joystick, 5));

        gamepad.Get_Left_Stick(axes.axes[Axis_Left_Horiz], axes.axes[Axis_Left_Vert]);
        gamepad.Get_Right_Stick(axes.axes[Axis_Right_Horiz], axes.axes[Axis_Right_Vert]);
        axes.axes[Axis_Left_Trigger]  = gamepad.Get_Axis(Joystick_Axes::LEFT_TRIGGER);
        axes.axes[Axis_Right_Trigger] = gamepad.Get_Axis(Joystick_Axes::RIGHT_TRIGGER);

        int num_keys;
        const uint8* keys = SDL_GetKeyboardState(&num_keys);
        axes.Update_Keys(keys, num_keys);
        //log_info("[%6d %6d] [%6d %6d] [%6d %6d]", left_stick_x, left_stick_y, right_stick_x, right_stick_y, left_trigger, right_trigger);

        ///////////////////////////////////////////////////////////////////////////////////////////
        double dt = (last_frame_time > 0.1) ? 0.1 : last_frame_time;
        Fixed_Update_And_Render(dt);
        
        // Draw version number
        Render_Text(renderer, g_small_font, font_color, {0,0,0,0}, "Version: %d", version_number);
        Render_Text(renderer, g_small_font, font_color, {0,g_font_size_small,0,0}, "Press [o]! :cat_smirk:");

        ///////////////////////////////////////////////////////////////////////////////////////////

        // draw frame counters
        SDL_Rect dest = {g_window_width-250, 0, 0, 0};
        Render_Text(renderer, g_small_font, font_color, dest, "Frame: #%llu", frame_number);
        dest.y += g_font_size_small;
        Render_Text(renderer, g_small_font, font_color, dest, "FPS: %5.1f", fps);
        dest.y += g_font_size_small;
        Render_Text(renderer, g_small_font, font_color, dest, "Avg FPS: %7.3f", fps_avg);

        uint64 WorkCounter = SDL_GetPerformanceCounter();
        real32 WorkSecondsElapsed = ((real64)(WorkCounter - LastCounter)) / ((real64)SDL_GetPerformanceFrequency());

        // TODO: Untested! buggy
        real32 SecondsElapsedForFrame = WorkSecondsElapsed;
        bool32 LockFramerate = true;
        if (LockFramerate) {
            if (SecondsElapsedForFrame < target_frame_time) {
                uint64 SleepMS = (uint64)(1000.0f*(target_frame_time - SecondsElapsedForFrame));
                if (SleepMS > 0) {
                    SDL_Delay(SleepMS);
                }

                while (SecondsElapsedForFrame < target_frame_time) {
                    SecondsElapsedForFrame = ((real64)(SDL_GetPerformanceCounter() - LastCounter)) / ((real64)SDL_GetPerformanceFrequency());
                }
            } else {
                // TODO: Missed Frame Rate!
                //RH_ERROR("Frame rate missed!");
            }
        }

        uint64 EndCounter = SDL_GetPerformanceCounter();
        last_frame_time = ((real64)(EndCounter - LastCounter)) / ((real64)SDL_GetPerformanceFrequency());
        real32 MSPerFrame = (real32)(1000.0f * last_frame_time);
        LastCounter = EndCounter;

        //log_trace("%7.3f ms  |  %9.5f fps  |  %9.5f fps_avg", MSPerFrame, fps, fps_avg);

        // Update the window display
	    SDL_RenderPresent(renderer);

        fps = 1000.0f / MSPerFrame;
        fps_avg = (fps_avg * (frame_number-1) + fps) / frame_number;
        if (frame_number < 5) fps_avg = fps;
    }
    log_info("Game event loop ended.");
    ////////////////////////////////////////////////////////////////////////////

    if (!Shutdown()) {
        log_fatal("Game::Run() Failed to shutdown");
        return false;
    }

    return true;
}

void Game_App::Goto_Main_Menu() {
    return_to_main = true;
}

void Game_App::Push_New_State(Game_State* state) {
    this->new_state = state;
}

void Game_App::Pop_State() {
    pop_state = true;
}

bool Game_App::Fixed_Update_And_Render(float dt) {
    //log_trace("Fixed_Update(%7.3f ms)", dt*1000.0f);

    //////////////////////////
    // Manage game_state
    if (return_to_main) {
        while (game_state.size() > 1) {
            Game_State* remove_state = this->game_state.top();
            delete remove_state;
            this->game_state.pop();
        }
        return_to_main = false;
        pop_state = false; // in case
    }
    if (pop_state) {
        Game_State* remove_state = this->game_state.top();
        delete remove_state;
        this->game_state.pop();
        pop_state = false;
    }
    if (new_state) {
        this->game_state.push(new_state);
        new_state = nullptr;
    }

    //////////////////////////
    // update
    Game_State* curr_state = this->game_state.top();
    if (curr_state) {
        curr_state->Update_And_Render(renderer, dt);
    }

    //////////////////////////
    // render
    {
        SDL_Rect dest = {g_window_width-180, g_window_height-g_font_size_small, 0, 0};
        SDL_Color font_color = {0, 0, 0};

        Render_Text(renderer, g_small_font, font_color, dest, "Stack Size: %d", game_state.size());
    }
    
    // Draw Cursor
    { 
        SDL_SetRenderDrawColor(renderer, 150, 200, 250, 255);
        SDL_Rect dest;
        dest.x = mouse.x;
        dest.y = mouse.y;
        dest.w = 32;
        dest.h = 32;
        SDL_RenderCopy(renderer, cursor, NULL, &dest);
    }

    // Draw joystick and input debug
    if (draw_input_debug) {
        if (SDL_NumJoysticks() > 0) {
            Draw_Gamepad();
        }

        {
            SDL_Rect dest = {0, g_window_height - 400, 0, 0};
            SDL_Color font_color = {0, 0, 0};

            Render_Text(renderer, g_small_font, font_color, dest, "Mouse1: %s", mouse[Mouse_Buttons::MOUSE1] ? "true" : "false");
            dest.y += g_font_size_small;
            Render_Text(renderer, g_small_font, font_color, dest, "Mouse2: %s", mouse[Mouse_Buttons::MOUSE2] ? "true" : "false");
            dest.y += g_font_size_small;
            Render_Text(renderer, g_small_font, font_color, dest, "Mouse3: %s", mouse[Mouse_Buttons::MOUSE3] ? "true" : "false");
            dest.y += g_font_size_small;
            Render_Text(renderer, g_small_font, font_color, dest, "Mouse4: %s", mouse[Mouse_Buttons::MOUSE4] ? "true" : "false");
            dest.y += g_font_size_small;
            Render_Text(renderer, g_small_font, font_color, dest, "Mouse5: %s", mouse[Mouse_Buttons::MOUSE5] ? "true" : "false");

            dest.y += g_font_size_small;
            if (mouse.wheel_y > 0)
                Render_Text(renderer, g_small_font, font_color, dest, "MWheelUp");
            if (mouse.wheel_y < 0)
                Render_Text(renderer, g_small_font, font_color, dest, "MWheelDown");
        }
    }

    return false;
}

SDL_Joystick* Game_App::Update_Joysticks() {
    static int num_joysticks = 0;
    int new_num = SDL_NumJoysticks();
    if (new_num != num_joysticks) {
        num_joysticks = new_num;
        log_warn("%d joysticks connected.", num_joysticks);

        for( int i=0; i < SDL_NumJoysticks(); i++ ) 
        {
            SDL_Joystick* joystick = SDL_JoystickOpen(i);
            SDL_JoystickID id = SDL_JoystickInstanceID(joystick);
            const char* joy_name = SDL_JoystickName(joystick);
            log_info("  Joystick #%d: '%s'[%d]", i, joy_name, id);

            return joystick;
        }
    }

    return SDL_JoystickOpen(0);
}

void Game_App::On_Action_Event(Action_Event event) {
    if (event.pressed) {
        //log_debug("Action_Event: %s: %s", Action_Event_String(event.action), event.pressed ? "Pressed" : "Released");

        if (this->game_state.top()->On_Action_Event(event)) {
            return;
        }
    }
}

void Game_App::On_Key_Event(int32 key_code, bool pressed) {
    if (key_code == SDLK_LEFTBRACKET && pressed) {
        SDL_Event quit_event;
        quit_event.type = SDL_QUIT;
        SDL_PushEvent(&quit_event);
        return;
    } else if (key_code == SDLK_o && pressed) {
        int channel = Mix_PlayChannel(-1, sound, 0);
        Mix_Volume(channel, 10);
        return;
    } else if (key_code == SDLK_c && pressed) {
        draw_input_debug = !draw_input_debug;
        return;
    }
}

void Game_App::Update_Volume(uint8 new_master_volume, uint8 new_music_volume) {
    int master_vol = (((real32)new_master_volume) * MIX_MAX_VOLUME / 100.0f);
    int music_vol = (real32)(new_music_volume*new_master_volume * MIX_MAX_VOLUME) / (10000.0f);

    Mix_MasterVolume(master_vol);
    Mix_VolumeMusic(music_vol);
}

void Game_App::Draw_Gamepad() {
    SDL_Color font_color = {0, 0, 0};
    
    SDL_Rect pad;
    pad.w = 24;
    pad.h = 24;

    pad.x = 0;
    pad.y = g_window_height-48;
    Render_Text(renderer, g_small_font, font_color, pad, "joystick Connected!");

    // D-Pad Left
    pad.x = 16;
    pad.y = g_window_height-170;
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    if (gamepad[DPAD_LEFT])
        SDL_RenderFillRect(renderer, &pad);
    else
        SDL_RenderDrawRect(renderer, &pad);
    
    // D-Pad Up
    pad.x += 32;
    pad.y -= 24;
    if (gamepad[DPAD_UP])
        SDL_RenderFillRect(renderer, &pad);
    else
        SDL_RenderDrawRect(renderer, &pad);
    
    // D-Pad Down
    pad.y += 48;
    if (gamepad[DPAD_DOWN])
        SDL_RenderFillRect(renderer, &pad);
    else
        SDL_RenderDrawRect(renderer, &pad);
    
    // D-Pad Right
    pad.x += 32;
    pad.y -= 24;
    if (gamepad[DPAD_RIGHT])
        SDL_RenderFillRect(renderer, &pad);
    else
        SDL_RenderDrawRect(renderer, &pad);

    // X Button
    SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
    pad.x += 64;
    if (gamepad[BUTTON_X])
        SDL_RenderFillRect(renderer, &pad);
    else
        SDL_RenderDrawRect(renderer, &pad);

    // Y Button
    SDL_SetRenderDrawColor(renderer, 100, 255, 255, 255);
    pad.x += 32;
    pad.y -= 24;
    if (gamepad[BUTTON_Y])
        SDL_RenderFillRect(renderer, &pad);
    else
        SDL_RenderDrawRect(renderer, &pad);
    
    // A Button
    SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
    pad.y += 48;
    if (gamepad[BUTTON_A])
        SDL_RenderFillRect(renderer, &pad);
    else
        SDL_RenderDrawRect(renderer, &pad);
    
    // B Button
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    pad.x += 32;
    pad.y -= 24;
    if (gamepad[BUTTON_B])
        SDL_RenderFillRect(renderer, &pad);
    else
        SDL_RenderDrawRect(renderer, &pad);
    
    // LB Button
    SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255);
    pad.x -= (128 + 64);
    pad.y -= 48;
    pad.w *= 2;
    pad.h *= 0.75;
    if (gamepad[LEFT_BUMPER])
        SDL_RenderFillRect(renderer, &pad);
    else
        SDL_RenderDrawRect(renderer, &pad);
    
    // RB Button
    pad.x += 128+40;
    if (gamepad[RIGHT_BUMPER])
        SDL_RenderFillRect(renderer, &pad);
    else
        SDL_RenderDrawRect(renderer, &pad);
    
    // Back Button
    pad.w = 24;
    pad.h = 16;
    pad.x -= 96;
    pad.y += 16;
    if (gamepad[BUTTON_BACK])
        SDL_RenderFillRect(renderer, &pad);
    else
        SDL_RenderDrawRect(renderer, &pad);
    
    // Start Button
    pad.x += 48;
    if (gamepad[BUTTON_START])
        SDL_RenderFillRect(renderer, &pad);
    else
        SDL_RenderDrawRect(renderer, &pad);

    // Left Analog Stick
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    pad.x -= 96;
    pad.y += 64+32;
    pad.w = 48;
    pad.h = 48;
    if (gamepad[Joystick_Buttons::BUTTON_LEFT_STICK])
        SDL_RenderFillRect(renderer, &pad);
    SDL_RenderDrawRect(renderer, &pad);
    Draw_Circle(renderer, pad.x + pad.w/2, pad.y + pad.h/2, deadzone*pad.h/2);

    SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
    int dx =  1.0*(0.5*gamepad.Get_Axis(Joystick_Axes::LEFT_STICK_X))*pad.w;
    int dy = -1.0*(0.5*gamepad.Get_Axis(Joystick_Axes::LEFT_STICK_Y))*pad.h;
    SDL_RenderDrawLine(renderer, pad.x+pad.w/2, pad.y+pad.h/2, pad.x+pad.w/2+dx, pad.y+pad.h/2+dy);

    // Right Analog Stick
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    pad.x += 128-8;
    if (gamepad[Joystick_Buttons::BUTTON_RIGHT_STICK])
        SDL_RenderFillRect(renderer, &pad);
    SDL_RenderDrawRect(renderer, &pad);
    Draw_Circle(renderer, pad.x + pad.w/2, pad.y + pad.h/2, deadzone*pad.h/2);

    SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
    dx =  1.0*(0.5*gamepad.Get_Axis(Joystick_Axes::RIGHT_STICK_X))*pad.w;
    dy = -1.0*(0.5*gamepad.Get_Axis(Joystick_Axes::RIGHT_STICK_Y))*pad.h;
    SDL_RenderDrawLine(renderer, pad.x+pad.w/2, pad.y+pad.h/2, pad.x+pad.w/2+dx, pad.y+pad.h/2+dy);

    // Left Trigger
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    pad.x -= 144;
    pad.y -= 170;
    pad.w *= 0.5;
    SDL_RenderDrawRect(renderer, &pad);
    pad.h = 48 * gamepad.Get_Axis(Joystick_Axes::LEFT_TRIGGER);
    SDL_RenderFillRect(renderer, &pad);

    // Right Trigger
    pad.x += 168+24;
    pad.h = 48;
    SDL_RenderDrawRect(renderer, &pad);
    pad.h = 48 * gamepad.Get_Axis(Joystick_Axes::RIGHT_TRIGGER);
    SDL_RenderFillRect(renderer, &pad);

    // Center Button
    pad.x -= 128-32;
    pad.y += 128;
    pad.w = 24;
    pad.h = 24;
    if (gamepad[BUTTON_CENTER])
        SDL_RenderFillRect(renderer, &pad);
    else
        SDL_RenderDrawRect(renderer, &pad);
}

bool Game_App::ReadConfig() {
    std::ifstream init_file("data/init.json");
    std::ifstream opt_file("config.json");
    try {
        json data = json::parse(init_file);
        log_info("Loading config from data/init.json");

        // Window Title
        if (data.find("Title") != data.end()) {
            window_name = data["Title"].get<std::string>();
        }
        
        // if not find width or height, use deafault
        if (data.find("Window_Width") != data.end()) {
            g_window_width = data["Window_Width"].get<int32>();
        }
        if (data.find("Window_Height") != data.end()) {
            g_window_height = data["Window_Height"].get<int32>();
        }

        // check if config file exists, and if so read options from that.
        if (opt_file.is_open()) {
            json conf = json::parse(opt_file);
            log_info("Loading options from config.json");

            // Master_Volume:
            if (conf.find("Options") != conf.end()) {

                json opt = conf["Options"];
                if (opt.find("Master_Volume") != opt.end()) {
                    options.master_volume = opt["Master_Volume"].get<int32>();
                } else if (data.find("Master_Volume") != data.end()) {
                    options.master_volume = data["Master_Volume"].get<int32>();
                }

                if (opt.find("Music_Volume") != opt.end()) {
                    options.music_volume = opt["Music_Volume"].get<int32>();
                } else if (data.find("Music_Volume") != data.end()) {
                    options.music_volume = data["Music_Volume"].get<int32>();
                }
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

    // check if ver.json exists
    std::ifstream ver_file("data/ver.json");
    version_number = 0;
    if (ver_file.is_open()) {
        json ver = json::parse(ver_file);

        if (ver.find("Version") != ver.end()) {
            version_number = ver["Version"].get<int32>();
        }
    }

    return true;
}

bool Game_App::WriteConfig() {
    // try to re-serialze to disk
    try {
        json data = {
            {"Options", {
                {"Master_Volume", options.master_volume},
                {"Music_Volume",  options.music_volume}
            }}
        };

        std::ofstream fp("config.json");
        fp << std::setw(2) << data << std::endl;
        fp.close();
        log_info("Options written to config.json");

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
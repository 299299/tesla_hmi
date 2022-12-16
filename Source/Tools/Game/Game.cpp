
#include "Game.h"

#include <SDL/SDL_hints.h>
#include <Urho3D/DebugNew.h>

// Movement speed as world units per second
const float MOVE_SPEED = 10.0f;
// Mouse sensitivity as degrees per pixel
const float MOUSE_SENSITIVITY = 0.1f;

const float CAMERA_MOVE_SPEED = 2.0;

const int START_X = 5;

const float CAR_LENGTH = 4.6;
const float CAR_WIDTH = 2.0;
const float CAR_HEIGHT = 1.5;
const float RADAR_TO_CAMERA = 1.52;

const float TAIL_LIGHT_X = 0;
const float TAIL_LIGHT_Y = 0.2;
const float TAIL_LIGHT_Z = -2.4;
const float TAIL_LIGHT_RANGE = 1.0;

const Color JADE(5 / 255.0, 196 / 255.0, 107 / 255.0);
const Color SUN_GLOW(250 / 255.0, 204 / 255.0, 72 / 255.0);
const Color NA_ORANGE(249 / 255.0, 151 / 255.0, 22 / 255.0);
const Color DARK_TURQ(0, 216 / 255.0, 216 / 255.0);
const Color SPRINT_GREEN(11 / 255.0, 232 / 255.0, 129 / 255.0);
const Color SOFT_BLUE(136 / 255.0, 225 / 255.0, 242 / 255.0);
const Color SOFT_PURPLE(147 / 255.0, 153 / 255.0, 255 / 255.0);

const Color AD_ON_COLOR(86 / 255.0, 164 / 255.0, 118 / 255.0);

static const int SET_IP = 2;
static const int GAME_INIT = 3;

const float turn_signal_flash_time = 0.5F;

enum CameraState
{
    kCameraFPS,
    kCameraTP,
    kCameraFixed,
};

Color gear_color(0.4, 0.4, 0.4);

enum GearEunm
{
    GEAR_P = 0,
    GEAR_D = 3,
    GEAR_N = 2,
    GEAR_R = 1,
};

URHO3D_DEFINE_APPLICATION_MAIN(Game);

#ifdef __ANDROID__
extern "C" {
extern int Android_JNI_SendMessage2(int cmd, double data1, double data2, double data3, double data4, double data5);
}
#endif

static void set_scale_by_car_size(Drawable* drawable)
{
    auto* node = drawable->GetNode();
    const auto& bb = drawable->GetWorldBoundingBox();
    auto size = bb.Size();
    // URHO3D_LOGINFOF("size=%s", size.ToString().CString());
    node->SetScale(Vector3(CAR_WIDTH / size.x_, CAR_HEIGHT / size.y_, CAR_LENGTH / size.z_));
}

static Game* g_game = NULL;

static void android_callback(int msg_Code, double data1, double data2, double data3, const char* data4)
{
    if (g_game)
        g_game->OnAndroidCallback(msg_Code, data1, data2, data3, data4);
}

static void apply_rotation(Node* node, float yaw)
{
    Quaternion q;
    // q.FromEulerAngles(0, 180, 0);
    q.FromEulerAngles(90, 180 + yaw, 90);
    node->SetRotation(q);
}

Game::Game(Context* context)
    : Sample(context),
      last_sync_time_(0.0F),
      op_status_(-1),
      camera_dist_(config_.camera_init_dist),
      yaw_(0),
      pitch_(config_.camera_init_pitch),
      message_time_(0),
      touch_down_time_(0),
      debug_touch_flag_(0),
      android_(false),
      num_cpu_cores_(0),
      turn_signal_time_(0),
      target_pitch_(-999.0F),
      target_dist_(-1.0F),
      target_yaw_(-999.0F),
      touch_up_time_(0.0F),
      camera_state_(kCameraTP),
      op_debug_mode_(0),
      camera_blend_speed_(0.1F),
      camera_blend_acceleration_(0.1F),
      status_text_time_out_(5.0F),
      debug_test_(false)
{
    URHO3D_LOGINFOF("************************ Game::Game() tid=%u ************************ ", pthread_self());
    g_game = this;
    SetFuncCallback(android_callback);
}

Game::~Game()
{
    g_game = NULL;
}

void Game::InitOP()
{
    URHO3D_LOGINFOF("************************ InitOP tid=%u ************************ ", pthread_self());

    URHO3D_LOGINFOF("%s", Time::GetTimeStamp().CString());

    const auto& arg_list = GetArguments();

    for (int i = 0; i < arg_list.Size(); ++i)
    {
        if (arg_list[i] == "-ip")
        {
            op_ip_address_ = arg_list[i + 1];
            URHO3D_LOGINFOF("command line ip=%s", op_ip_address_.CString());
        }
        else if (arg_list[i] == "test")
        {
            debug_test_ = true;
        }
    }

    auto vec = op_ip_address_.Split('.');
    bool ping_successed = false;

    if (vec.Size() != 4)
    {
#ifdef __ANDROID__
        const char* ip_list[] = {"192.168.43.138", "192.168.137.138"};
#else
        const char* ip_list[] = {"192.168.1.17", "192.168.43.138", "127.0.0.1"};
#endif
        for (int i = 0; i < sizeof(ip_list) / sizeof(ip_list[0]); ++i)
        {
            std::string details;
            if (Ping(ip_list[i], 1, details))
            {
                URHO3D_LOGINFOF("ping %s success!!!", ip_list[i]);
                op_ip_address_ = ip_list[i];
                ping_successed = true;
                break;
            }
            else
            {
                URHO3D_LOGINFOF("ping %s failed !!! %s ", ip_list[i], details.c_str());
            }
        }
    }
    else
    {
        URHO3D_LOGINFO("input ip=" + op_ip_address_);
        std::string details;
        ping_successed = Ping(op_ip_address_.CString(), 3, details);
    }

    op_status_ = ping_successed ? 0 : -1;
    URHO3D_LOGINFOF("op_status_=%d", op_status_);

    op_ctx_ = OP::Context::create();

    const char* sock_names[] = {
        "remote_comm",
    };
    auto size = (sizeof(sock_names) / sizeof(sock_names[0]));
    for (int i = 0; i < size; ++i)
    {
        auto* sock = OP::SubSocket::create(op_ctx_, sock_names[i], op_ip_address_.CString(), true);
        if (sock == NULL)
            URHO3D_LOGERRORF("%s error !!! \n", sock_names[i]);
        else
            socks_.push_back(sock);
    }

    sync_pub_ = OP::PubSocket::create(op_ctx_, "remote_comm_ctrl");
    op_poller_ = OP::Poller::create(socks_);
    last_sync_time_ = time_->GetElapsedTime();
}

void Game::Start()
{
    input_ = GetSubsystem< Input >();
    graphics_ = GetSubsystem< Graphics >();
    time_ = GetSubsystem< Time >();
    ui_ = GetSubsystem< UI >();
    render_ = GetSubsystem< Renderer >();
    cache_ = GetSubsystem< ResourceCache >();

    Sample::Start();
    CreateScene();

    if (touchEnabled_)
        input_->SetScreenJoystickVisible(screenJoystickIndex_, false);

    SetHDR(config_.hdr);
    CreateUI();
    InitOP();

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Game, HandleSceneUpdate));
    SubscribeToEvent(E_TOUCHEND, URHO3D_HANDLER(Game, HandleTouchEnd));
    SubscribeToEvent(E_UIMOUSECLICK, URHO3D_HANDLER(Game, HandleControlClicked));
    SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(Game, HandleMouseButtonUp));

    message_time_ = time_->GetElapsedTime();
    android_ = (GetPlatform() == "Android");
    num_cpu_cores_ = GetNumPhysicalCPUs();

#ifdef __ANDROID__
    Android_JNI_SendMessage2(1, 0, 0, 0, 0, 0);
#endif

    URHO3D_LOGINFOF("Game---start finished");
}

void Game::SetHDR(bool hdr)
{
    SharedPtr< Viewport > viewport(new Viewport(context_, scene_, cameraNode_->GetComponent< Camera >()));
    render_->SetViewport(0, viewport);

    UpdateViewport();

    bool is_low_end = android_ && (num_cpu_cores_ < 6);
    if (is_low_end)
        return;

    render_path_ = viewport->GetRenderPath()->Clone();
    if (hdr)
    {
#ifdef __ANDROID__
        render_path_->Load(cache_->GetResource< XMLFile >("RenderPaths/ForwardDepth.xml"));
#else
        render_path_->Load(cache_->GetResource< XMLFile >("RenderPaths/Forward.xml"));
#endif
        render_path_->Append(cache_->GetResource< XMLFile >("PostProcess/AutoExposure.xml"));
        render_path_->Append(cache_->GetResource< XMLFile >("PostProcess/BloomHDR.xml"));
        render_path_->Append(cache_->GetResource< XMLFile >("PostProcess/Tonemap.xml"));
        render_path_->SetEnabled("TonemapReinhardEq3", false);
        render_path_->SetEnabled("TonemapUncharted2", true);
        render_path_->SetShaderParameter("TonemapMaxWhite", 1.8F);
        render_path_->SetShaderParameter("TonemapExposureBias", 2.5F);
        render_path_->SetShaderParameter("AutoExposureAdaptRate", 2.0F);
        render_path_->SetShaderParameter("BloomHDRMix", Variant(Vector2(0.9f, 0.6f)));
        render_path_->Append(cache_->GetResource< XMLFile >("PostProcess/FXAA2.xml"));
        // render_path_->Append(cache_->GetResource<XMLFile>("PostProcess/ColorCorrection.xml"));
        viewport->SetRenderPath(render_path_);
        render_->SetHDRRendering(true);
    }
    else
    {
        render_path_->Load(cache_->GetResource< XMLFile >("RenderPaths/Forward.xml"));
        render_path_->Append(cache_->GetResource< XMLFile >("PostProcess/FXAA2.xml"));
        // render_path_->Append(cache_->GetResource<XMLFile>("PostProcess/ColorCorrection.xml"));
        viewport->SetRenderPath(render_path_);
    }
}

void Game::Stop()
{
    URHO3D_LOGINFO("**************** Stop **************** ");
    delete op_poller_;
    op_poller_ = NULL;
    delete sync_pub_;
    sync_pub_ = NULL;
    for (auto* p : socks_)
    {
        delete p;
    }
    socks_.clear();

    g_game = NULL;
    SetFuncCallback(NULL);
}

void Game::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    Update(timeStep);
}

void Game::Update(float timeStep)
{
    ReceiveDataFromOP();
    UpdateInput(timeStep);
    SyncUI(timeStep);
    SyncToOP();
}

void Game::ReceiveDataFromOP()
{
    auto cur_time = time_->GetElapsedTime();
    if (cur_time - message_time_ > 5.0 && op_status_ == 1)
    {
        URHO3D_LOGERROR("message time out");
        op_status_ = 2;
    }
    // peek and consume all events in the zmq queue, then return.
    auto polls = op_poller_->poll(10);

    if (polls.size() == 0)
        return;

    for (auto sock : polls)
    {
        // URHO3D_LOGINFO("start sock receive");
        OP::Message* msg = sock->receive();
        if (msg == NULL)
        {
            URHO3D_LOGINFO("receive NULL msg");
            continue;
        }

        auto size = msg->getSize();
        json_buffer_.Resize(size + 1);
        char* p = &json_buffer_[0];
        memset(p, 0, size + 1);
        memcpy(p, msg->getData(), size);
        String s(p);
        URHO3D_LOGINFO(s);
        SharedPtr< JSONFile > json(new JSONFile(context_));
        if (json->FromString(s))
            HandleCustomMessage(json);

        delete msg;
    }
}

void Game::SyncToOP()
{
    auto elapsed = time_->GetElapsedTime();

    if (elapsed - last_sync_time_ > 0.025F)
    {
        sync_str_ = "{";
        sync_str_ += "\"test\":" + String(0) + ",";
        sync_str_ += "}";
        auto ret = sync_pub_->send((char*)sync_str_.CString(), sync_str_.Length());
        // URHO3D_LOGINFOF("sync to op %s ", buf_to_send);
        last_sync_time_ = elapsed;
    }
}

void Game::CreateScene()
{
    scene_ = new Scene(context_);
    scene_->CreateComponent< Octree >();
    scene_->CreateComponent< DebugRenderer >();

    auto* zoneNode = scene_->CreateChild("Zone");
    auto* zone = zoneNode->CreateComponent< Zone >();
    const float WORLD_SIZE = 500.0F;
    // Set same volume as the Octree, set a close bluish fog and some ambient light
    zone->SetBoundingBox(BoundingBox(-WORLD_SIZE, WORLD_SIZE));
    zone->SetFogStart(50.0f);
    zone->SetFogEnd(150.0f);
    zone_ = zone;

    Node* planeNode = scene_->CreateChild("Plane");
    planeNode->SetScale(Vector3(200.0f, 1.0f, 200.0f));
    planeNode->SetPosition(Vector3(0, -0.1, 0));
    auto* planeObject = planeNode->CreateComponent< StaticModel >();
    planeObject->SetModel(cache_->GetResource< Model >("Models/Plane.mdl"));
    if (config_.water_plane)
        planeObject->SetMaterial(cache_->GetResource< Material >("Materials/RefPlane.xml"));

    cameraNode_ = scene_->CreateChild("Camera");
    auto* camera = cameraNode_->CreateComponent< Camera >();
    cameraNode_->SetPosition(Vector3(0, 11.5, -15.5));
    // cameraNode_->LookAt(Vector3(0, 0, 0));
    camera->SetFarClip(500.0f);

    auto* lightNode = scene_->CreateChild("Light");
    auto* light = lightNode->CreateComponent< Light >();
    light->SetLightType(LIGHT_SPOT);
    light->SetRange(30.0);
    lightNode->SetPosition(Vector3(0, 0.5, 0.5));
    lightNode->SetDirection(Vector3(0, 0, 1.0F));
    light->SetSpecularIntensity(10.0f);
    light->SetFov(45.0);

    if (config_.shadow)
    {
        light->SetCastShadows(true);
        light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
        // Set cascade splits at 10, 50 and 200 world units, fade shadows out at 80% of maximum shadow distance
        light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
    }
    front_light_ = light;

    /// reference
    // auto* boxNode = scene_->CreateChild("Box_Ref");
    // boxNode->SetPosition(Vector3(5,CAR_HEIGHT/2.0,0));
    // auto* boxObject = boxNode->CreateComponent<StaticModel>();
    // boxObject->SetModel(cache_->GetResource<Model>("Models/Box.mdl"));
    // set_scale_by_car_size(boxObject);

    ego_node_ = CreateCarModel("EgoCar", "MY/lexus.xml");
    ego_node_->SetEnabled(true);
    ego_node_->SetPosition(Vector3::ZERO);
    ego_mat_ = cache_->GetResource< Material >("MY/lexus.xml");

    {
        auto* node = scene_->CreateChild("TailLight");
        node->SetPosition(Vector3(TAIL_LIGHT_X, TAIL_LIGHT_Y, TAIL_LIGHT_Z));
        auto* light = node->CreateComponent< Light >();
        light->SetLightType(LIGHT_POINT);
        light->SetRange(TAIL_LIGHT_RANGE);
        light->SetColor(Color::RED);
        light->SetEnabled(false);
        tail_light_ = light;
    }

    line_mat_ = cache_->GetResource< Material >("MY/Lane.xml");
    UpdateDayLight();
}

void Game::UpdateInput(float timeStep)
{
    // Do not move if the UI has a focused element (the console)
    if (ui_->GetFocusElement())
        return;

    // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
    IntVector2 mouseMove = input_->GetMouseMove();
    yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
    pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
    pitch_ = Clamp(pitch_, -90.0f, 90.0f);

    if (input_->GetNumTouches() == 1)
    {
        auto* state = input_->GetTouch(0);
        if (!state->touchedElement_)  // Touch on empty space
        {
            if (state->delta_.x_ || state->delta_.y_)
            {
                Camera* camera = cameraNode_->GetComponent< Camera >();
                if (camera)
                {
                    yaw_ += TOUCH_SENSITIVITY * camera->GetFov() / graphics_->GetHeight() * state->delta_.x_;
                    pitch_ += TOUCH_SENSITIVITY * camera->GetFov() / graphics_->GetHeight() * state->delta_.y_;
                }
            }
        }
    }

    UpdateDebugTouch(timeStep);

    int target_cam_state = -1;
    if (car_status_.gear == GEAR_P || car_status_.gear == GEAR_N)
        target_cam_state = kCameraFixed;
    else if (car_status_.gear == GEAR_R || car_status_.gear == GEAR_D)
        target_cam_state = kCameraTP;

    if (target_cam_state != camera_state_)
    {
        camera_state_ = target_cam_state;

        if (target_cam_state == kCameraFixed)
        {

        }
        else if (target_cam_state == kCameraTP)
        {
            target_pitch_ = config_.camera_init_pitch_tp;
        }
    }

    if (camera_state_ == kCameraFPS)
        UpdateFPSCamera(timeStep);
    else if (camera_state_ == kCameraTP)
        UpdateTPCamera(timeStep);
    else
        UpdateFixedCamera(timeStep);

    UpdateKeyInput(timeStep);
}

void Game::UpdateDayLight()
{
    bool is_day = !config_.is_night;
    Color day_color(config_.day_color[0] / 255.0, config_.day_color[1] / 255.0, config_.day_color[2] / 255.0);
    Color night_color(config_.night_color[0] / 255.0, config_.night_color[1] / 255.0, config_.night_color[2] / 255.0);

    Color ambient_color = is_day ? day_color : night_color;
    zone_->SetAmbientColor(ambient_color);
    zone_->SetFogColor(ambient_color);
    front_light_->SetColor(Color::WHITE);
    // tail_light_->SetEnabled(!is_day);

    Vector4 ego_color(0.1, 0.1, 0.1, 1);
    if (!is_day)
        ego_color = Vector4(0.25, 0.25, 0.25, 1);
    ego_mat_->SetShaderParameter("MatEmissiveColor", ego_color);
}

void Game::CreateUI()
{
    SetLogoVisible(false);
    // Load XML file containing default UI style sheet
    auto* style = cache_->GetResource< XMLFile >("UI/DefaultStyle.xml");
    auto uiRoot = ui_->GetRoot();
    auto vw = graphics_->GetWidth();
    auto vh = graphics_->GetHeight();
    auto half_w = vw / 2.0F;
    auto half_h = vh / 2.0F;

    URHO3D_LOGINFOF("vw=%d vh=%d \n", vw, vh);

    // Set the loaded style as default style
    uiRoot->SetDefaultStyle(style);

    // const String FONT_NAME("MY/GAEN.ttf");
    // const String FONT_NAME("MY/Google.ttf");
    const String FONT_NAME("MY/SimHei.ttf");
    auto* font = cache_->GetResource< Font >(FONT_NAME);

    debug_text_ = uiRoot->CreateChild< Text >("debug");
    debug_text_->SetText("");
    debug_text_->SetFont(font, android_ ? config_.debug_text_size : 30);
    debug_text_->SetColor(Color::RED);
    debug_text_->SetHorizontalAlignment(HA_LEFT);
    debug_text_->SetVerticalAlignment(VA_TOP);
    debug_text_->SetPriority(100000);

    speed_text_ = uiRoot->CreateChild< Text >("speed");
    speed_text_->SetTextEffect(TE_STROKE);
    speed_text_->SetFont(font, config_.speed_text_size);
    speed_text_->SetColor(DARK_TURQ);
    ui_elements_.Push(speed_text_);

    speed_hint_text_ = uiRoot->CreateChild< Text >("speed_hint");
    speed_hint_text_->SetFont(font, config_.speed_text_size / 6.0);
    speed_hint_text_->SetColor(Color(0.5, 0.5, 0.5));
    speed_hint_text_->SetPriority(-10);
    ui_elements_.Push(speed_hint_text_);

    status_text_ = uiRoot->CreateChild< Text >("status");
    status_text_->SetTextEffect(TE_SHADOW);
    status_text_->SetFont(font, config_.status_text_size);
    status_text_->SetColor(Color(26 / 255.0, 80 / 255.0, 139 / 255.0));
    status_text_->SetText("");
    ui_elements_.Push(status_text_);

    int icon_top = config_.top_gap + config_.icon_size / 2.0;
    {
        left_turn_signal_sprite_ = uiRoot->CreateChild< Sprite >("left_turn_signal");
        left_turn_signal_sprite_->SetTexture(cache_->GetResource< Texture2D >("MY/arrow_l.png"));
        left_turn_signal_sprite_->SetSize(config_.indicator_size, config_.indicator_size);
        left_turn_signal_sprite_->SetHotSpot(config_.indicator_size / 2.0, config_.indicator_size / 2.0);
        left_turn_signal_sprite_->SetOpacity(0.9f);
        left_turn_signal_sprite_->SetVisible(false);
        left_turn_signal_sprite_->SetPosition(
            config_.left_gap + config_.icon_size + config_.normal_gap + config_.indicator_size / 2.0, icon_top);
        ui_elements_.Push(left_turn_signal_sprite_);
    }

    {
        right_turn_signal_sprite_ = uiRoot->CreateChild< Sprite >("right_turn_signal");
        right_turn_signal_sprite_->SetTexture(cache_->GetResource< Texture2D >("MY/arrow_r.png"));
        right_turn_signal_sprite_->SetSize(config_.indicator_size, config_.indicator_size);
        right_turn_signal_sprite_->SetHotSpot(config_.indicator_size / 2.0, config_.indicator_size / 2.0);
        right_turn_signal_sprite_->SetOpacity(0.9f);
        right_turn_signal_sprite_->SetVisible(false);
        right_turn_signal_sprite_->SetPosition(
            vw - config_.right_gap - config_.icon_size - config_.normal_gap - config_.indicator_size / 2.0, icon_top);
        ui_elements_.Push(right_turn_signal_sprite_);
    }

    {
        ad_on_sprite_ = uiRoot->CreateChild< Sprite >("ad_on");
        ad_on_sprite_->SetTexture(cache_->GetResource< Texture2D >("MY/ad_on.png"));
        ad_on_sprite_->SetSize(config_.icon_size, config_.icon_size);
        ad_on_sprite_->SetHotSpot(config_.icon_size / 2.0, config_.icon_size / 2.0);
        ad_on_sprite_->SetOpacity(0.9f);
        ad_on_sprite_->SetVisible(false);
        ad_on_sprite_->SetPosition(config_.left_gap + config_.icon_size / 2.0, icon_top);
        ui_elements_.Push(ad_on_sprite_);
    }

    {
        ad_off_sprite_ = uiRoot->CreateChild< Sprite >("ad_off");
        ad_off_sprite_->SetTexture(cache_->GetResource< Texture2D >("MY/ad_off.png"));
        ad_off_sprite_->SetSize(config_.icon_size, config_.icon_size);
        ad_off_sprite_->SetHotSpot(config_.icon_size / 2.0, config_.icon_size / 2.0);
        ad_off_sprite_->SetOpacity(0.9f);
        ad_off_sprite_->SetVisible(false);
        ad_off_sprite_->SetPosition(config_.left_gap + config_.icon_size / 2.0, icon_top);
    }

    debug_setting_btn_ = uiRoot->CreateChild< Text >("debug_setting");
    debug_setting_btn_->SetFont(font, config_.debug_ui_size);
    debug_setting_btn_->SetText("setting");
    debug_setting_btn_->SetColor(Color::RED);
    debug_setting_btn_->SetVisible(false);
    debug_ui_elements_.Push(debug_setting_btn_);

    debug_log_btn_ = uiRoot->CreateChild< Text >("debug_log_btn");
    debug_log_btn_->SetFont(font, config_.debug_ui_size);
    debug_log_btn_->SetText("log");
    debug_log_btn_->SetVisible(false);
    debug_log_btn_->SetColor(Color::RED);
    debug_ui_elements_.Push(debug_log_btn_);

    debug_clean_data_btn_ = uiRoot->CreateChild< Text >("debug_clean_data_btn");
    debug_clean_data_btn_->SetFont(font, config_.debug_ui_size);
    debug_clean_data_btn_->SetText("clean");
    debug_clean_data_btn_->SetVisible(false);
    debug_clean_data_btn_->SetColor(Color::RED);
    debug_ui_elements_.Push(debug_clean_data_btn_);

    debug_no_log_btn_ = uiRoot->CreateChild< Text >("debug_no_log_btn");
    debug_no_log_btn_->SetFont(font, config_.debug_ui_size);
    debug_no_log_btn_->SetText("no log");
    debug_no_log_btn_->SetVisible(false);
    debug_no_log_btn_->SetColor(Color::RED);
    debug_ui_elements_.Push(debug_no_log_btn_);

    auto left = 30;
    auto top = vh / 2.0F;
    auto gap = 30;
    debug_setting_btn_->SetPosition(left, top);
    debug_log_btn_->SetPosition(left + debug_setting_btn_->GetRowWidth(0) + gap, top);
    debug_clean_data_btn_->SetPosition(left, top + debug_setting_btn_->GetRowHeight() + gap);
    debug_no_log_btn_->SetPosition(left + debug_clean_data_btn_->GetRowWidth(0) + gap,
                                   top + debug_setting_btn_->GetRowHeight() + gap);

    gear_p_text_ = uiRoot->CreateChild< Text >("P");
    gear_p_text_->SetFont(font, config_.gear_ui_size);
    gear_p_text_->SetText("P");
    gear_p_text_->SetColor(gear_color);

    int gear_w_offset = 10;
    int gear_h_offset = 10;
    auto gear_h = gear_p_text_->GetRowHeight();
    auto gear_w = gear_p_text_->GetRowWidth(0);
    gap = 10;
    auto gear_x = gear_w_offset;
    auto gear_y = vh - gear_h - gear_h_offset;
    gear_p_text_->SetPosition(gear_x, gear_y);
    ui_elements_.Push(gear_p_text_);

    gear_x += (gap + gear_w);
    gear_r_text_ = uiRoot->CreateChild< Text >("R");
    gear_r_text_->SetFont(font, config_.gear_ui_size);
    gear_r_text_->SetText("R");
    gear_r_text_->SetColor(gear_color);
    gear_r_text_->SetPosition(gear_x, gear_y);
    ui_elements_.Push(gear_r_text_);

    gear_x += (gap + gear_w);
    gear_n_text_ = uiRoot->CreateChild< Text >("N");
    gear_n_text_->SetFont(font, config_.gear_ui_size);
    gear_n_text_->SetText("N");
    gear_n_text_->SetPosition(gear_x, gear_y);
    gear_n_text_->SetColor(gear_color);
    ui_elements_.Push(gear_n_text_);

    gear_x += (gap + gear_w);
    gear_d_text_ = uiRoot->CreateChild< Text >("D");
    gear_d_text_->SetFont(font, config_.gear_ui_size);
    gear_d_text_->SetText("D");
    gear_d_text_->SetPosition(gear_x, gear_y);
    gear_d_text_->SetColor(gear_color);
    ui_elements_.Push(gear_d_text_);

    for (auto ui : debug_ui_elements_)
        ui_elements_.Push(ui);

    // for (auto ui : ui_elements_)
    // {
    //     URHO3D_LOGINFOF("%s pos=%s", ui->GetName().CString(), ui->GetPosition().ToString().CString());
    // }
}

void Game::UpdateViewport()
{
    auto* vp = render_->GetViewport(0);
    auto gw = graphics_->GetWidth();
    auto gh = graphics_->GetHeight();
    IntRect viewport_rect(0, 0, gw, gh);
    vp->SetRect(viewport_rect);
}

Node* Game::CreateCarModel(const char* name, const char* mat_name)
{
    auto* node = scene_->CreateChild(name);
    auto* car_model = node->CreateComponent< StaticModel >();
    car_model->SetModel(cache_->GetResource< Model >("MY/lexus.mdl"));
    car_model->SetMaterial(cache_->GetResource< Material >(mat_name));
    Quaternion q;
    // q.FromEulerAngles(0, 180, 0);
    q.FromEulerAngles(90, 180, 90);
    node->SetRotation(q);
    set_scale_by_car_size(car_model);
    node->SetEnabled(false);
    return node;
}

void Game::UpdateFPSCamera(float dt)
{
    Quaternion q(pitch_, yaw_, 0.0f);
    // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
    cameraNode_->SetRotation(q);
    // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
    if (input_->GetKeyDown(KEY_A))
        cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * dt);
    if (input_->GetKeyDown(KEY_D))
        cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * dt);
    if (input_->GetKeyDown(KEY_W))
        cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * dt);
    if (input_->GetKeyDown(KEY_S))
        cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * dt);
}

void Game::UpdateFixedCamera(float dt)
{
    camera_blend_speed_ = 0.05F;

    Quaternion q(config_.camera_init_pitch, 0, 0.0f);
    Vector3 target_pos = Vector3(0, 2.0, 1.0);
    Vector3 eye_pos = q * Vector3(0, 10.0, -3.0) + target_pos;

    auto cur_eye_pos = cameraNode_->GetPosition();
    auto diff = (eye_pos - cur_eye_pos) * 0.1F;

    cameraNode_->SetPosition(cur_eye_pos + diff);
    cameraNode_->LookAt(target_pos);
}

void Game::UpdateTPCamera(float dt)
{
    camera_dist_ -= input_->GetMouseMoveWheel();

    if (input_->GetNumTouches() == 0)
        touch_up_time_ += dt;
    else
    {
        touch_up_time_ = 0.0F;
        target_pitch_ = -999.0F;
        target_yaw_ = -999.0F;
        target_dist_ = -1.0F;
    }

    if (touch_up_time_ > config_.camera_reset_time || car_status_.speed_kmh > 0.0)
    {
        target_pitch_ = config_.camera_init_pitch_tp;
        target_dist_ = config_.camera_init_dist;
        target_yaw_ = 0.0F;
        touch_up_time_ = 0.0F;
    }

    // Zoom in/out
    if (input_->GetNumTouches() == 2)
    {
        bool zoom = false;

        TouchState* touch1 = input_->GetTouch(0);
        TouchState* touch2 = input_->GetTouch(1);

        // Check for zoom pattern (touches moving in opposite directions and on empty space)
        if (!touch1->touchedElement_ && !touch2->touchedElement_ &&
            ((touch1->delta_.y_ > 0 && touch2->delta_.y_ < 0) || (touch1->delta_.y_ < 0 && touch2->delta_.y_ > 0)))
        {
            zoom = true;
        }
        else
            zoom = false;

        if (zoom)
        {
            int sens = 0;
            // Check for zoom direction (in/out)
            if (Abs(touch1->position_.y_ - touch2->position_.y_) >
                Abs(touch1->lastPosition_.y_ - touch2->lastPosition_.y_))
                sens = -1;
            else
                sens = 1;
            camera_dist_ += Abs(touch1->delta_.y_ - touch2->delta_.y_) * sens * TOUCH_SENSITIVITY / 50.0f;
        }
    }

    if (target_pitch_ >= -100.0F)
    {
        auto diff = target_pitch_ - pitch_;
        pitch_ += diff * CAMERA_MOVE_SPEED * dt;
        if (std::abs(diff) < 0.1)
        {
            target_pitch_ = -999.0F;
        }
    }

    if (target_dist_ >= 0.0F)
    {
        auto diff = target_dist_ - camera_dist_;
        camera_dist_ += diff * CAMERA_MOVE_SPEED * dt;
        if (std::abs(diff) < 0.1)
        {
            target_dist_ = -1.0F;
        }
    }

    if (target_yaw_ >= -100.0F)
    {
        auto diff = target_yaw_ - yaw_;
        yaw_ += diff * CAMERA_MOVE_SPEED * dt;
        if (std::abs(diff) < 0.1)
        {
            target_yaw_ = -999.0F;
        }
    }

    pitch_ = Clamp(pitch_, -90.0f, 90.0f);
    camera_dist_ = Clamp(camera_dist_, config_.camera_min_dist, config_.camera_max_dist);

    Quaternion q(pitch_, yaw_, 0.0f);
    Vector3 target_pos = Vector3(0, 2.0, 1.0);
    Vector3 eye_pos = q * Vector3(0, 0, -camera_dist_) + target_pos;

    camera_blend_speed_ += camera_blend_acceleration_ * dt;
    camera_blend_speed_ = Min(1.0F, camera_blend_speed_);

    auto cur_eye_pos = cameraNode_->GetPosition();
    auto diff = (eye_pos - cur_eye_pos) * camera_blend_speed_;

    cameraNode_->SetPosition(cur_eye_pos + diff);
    cameraNode_->LookAt(target_pos);
}

void Game::UpdateDebugTouch(float dt)
{
    if (input_->GetNumTouches() == 1)
    {
        auto* state = input_->GetTouch(0);
        auto x = state->position_.x_;
        auto y = state->position_.y_;

        touch_down_time_ += dt;
        if (touch_down_time_ >= 3.0 && debug_touch_flag_ == 0)
        {
            auto w = graphics_->GetWidth();
            auto h = graphics_->GetHeight();

            if (y < h / 4)
            {
                // top area
                if (x < w / 4)
                {
                    // top left
                    debug_touch_flag_ = 1;
                }
                else if (x > w / 4 * 3)
                {
                    // top right
                    debug_touch_flag_ = 2;
                }
            }
            else if (y > h / 4 * 3)
            {
                // bottom area
                if (x < w / 4)
                {
                    // bottom left
                    debug_touch_flag_ = 3;
                }
                else if (x > w / 4 * 3)
                {
                    // bottom right
                    debug_touch_flag_ = 4;
                }
            }
            else
            {
            }

            URHO3D_LOGINFOF("debug_touch_flag_=%d \n", debug_touch_flag_);

            if (debug_touch_flag_ == 1)
            {
                // up left
                config_.debug = !config_.debug;
            }
            else if (debug_touch_flag_ == 3)
            {
                // bottom left
                if (op_debug_mode_ == 0)
                    op_debug_mode_ = 1;
                else if (op_debug_mode_ == 1)
                    op_debug_mode_ = 0;
            }
            else if (debug_touch_flag_ == 4)
            {
                // bottom right
                car_status_.turn_signal++;
                car_status_.turn_signal = car_status_.turn_signal % 4;
            }
        }
    }
    else
    {
        touch_down_time_ = 0.0;
        debug_touch_flag_ = 0;
    }
}

void Game::OnAndroidCallback(int msg, double data1, double data2, double data3, const char* data4)
{
    // URHO3D_LOGINFOF("OnAndroidCallback id=%d, (%f,%f,%f,%s) ", msg, data1, data2, data3, data4);
    switch (msg)
    {
        case SET_IP:
            URHO3D_LOGINFOF("SET_IP %f %f %f %s ", data1, data2, data3, data4);
            op_ip_address_ = data4;
            break;
        case GAME_INIT:
            URHO3D_LOGINFOF("game init %f %f %f %s ", data1, data2, data3, data4);
            break;
    }
}

void Game::SyncUI(float timeStep)
{
    UpdateViewport();
    Draw3D(timeStep);
    Draw2D(timeStep);

    for (auto ui : debug_ui_elements_)
        ui->SetVisible(config_.debug);

    if (config_.debug)
        DrawDebug();
    else
        debug_text_->SetText("");
}

void Game::DrawDebug()
{
    Urho3D::String info_text = " camera-dist: " + String(camera_dist_) + " pitch=" + String(pitch_) +
                               " cpu-core=" + String(num_cpu_cores_) + " \n";
    char buf[256] = {0};
    Urho3D::String text;
    if (op_status_ == 1)
    {
        // text += String(buf);
    }
    else if (op_status_ == 2)
    {
        info_text += String("message time out !!!");
    }
    else if (op_status_ == 0)
    {
        info_text += String(" waiting for connection");
    }
    else if (op_status_ == -1)
    {
        info_text += op_ip_address_ + String(" ping failed");
    }

    info_text += text;
    // info_text += "\n graphics w=" + String(vw) + " h=" + String(vh);

    debug_text_->SetText(info_text);

    if (config_.debug)
    {
        DebugRenderer* debug = scene_->GetComponent< DebugRenderer >();
        Sphere sp(Vector3::ZERO, 0.25F);
        debug->AddSphere(sp, Color::RED, false);
        front_light_->DrawDebugGeometry(debug, true);
        tail_light_->DrawDebugGeometry(debug, true);

        debug->AddNode(ego_node_->GetParent(), 2.0, false);

        for (auto ele : ui_elements_)
        {
            ui_->DebugDraw(ele);
        }
        for (auto slot : car_status_.slots)
        {
            for (auto p : slot.points)
            {
                Sphere sp(p, 0.1F);
                debug->AddSphere(sp, Color::YELLOW, false);
            }
        }
    }

    // auto* cam = cameraNode_->GetComponent< Camera >();
    // cam->SetFillMode(config_.debug ? FILL_WIREFRAME : FILL_SOLID);
}

void Game::Draw3D(float dt)
{
    tail_light_->SetEnabled(car_status_.brake_lights);

    for (auto n : slot_nodes_)
    {
        auto g = n->GetComponent< CustomGeometry >();
        g->Clear();
        g->Commit();
    }

    while (car_status_.slots.size() > slot_nodes_.size())
    {
        auto node = scene_->CreateChild("slot");
        node->CreateComponent< CustomGeometry >();
        slot_nodes_.push_back(node);
    }

    int num = car_status_.slots.size();
    for (int i = 0; i < num; ++i)
    {
        const auto& slot = car_status_.slots[i];
        auto* n = slot_nodes_[i];
        auto* g = n->GetComponent< CustomGeometry >();

        g->Clear();
        g->SetNumGeometries(1);
        g->SetDynamic(true);

        g->BeginGeometry(0, TRIANGLE_STRIP);

        g->DefineVertex(slot.points[0]);
        g->DefineVertex(slot.points[1]);
        g->DefineVertex(slot.points[3]);
        g->DefineVertex(slot.points[2]);

        g->SetMaterial(line_mat_);
        g->Commit();
    }
}

void Game::Draw2D(float dt)
{
    auto* vp = render_->GetViewport(0);
    auto view_rect = vp->GetRect();
    auto vw = view_rect.Width();
    auto vh = view_rect.Height();
    auto half_w = vw / 2;
    auto half_h = vh / 2;

    if (op_status_ == 1)
    {
        status_text_time_out_ -= dt;
        if (status_text_time_out_ <= 0.0F)
        {
            status_text_->SetText("");
            status_text_time_out_ = 0.0F;
        }
        String text = String((int)car_status_.speed_kmh);
        speed_text_->SetText(text);
        speed_hint_text_->SetText("km/h");
    }
    else if (op_status_ == 2)
    {
        status_text_->SetText("message time out !!!      ");
        status_text_time_out_ = 5.0F;
    }
    else if (op_status_ == -1)
    {
        status_text_->SetText(op_ip_address_ + String(" ping failed"));
        status_text_time_out_ = 5.0F;
    }
    else if (op_status_ == 0)
    {
        status_text_->SetText("CONNECTING");
        status_text_time_out_ = 5.0F;
    }

    auto width = speed_text_->GetRowWidth(0);
    auto height = speed_text_->GetRowHeight();
    auto top = 10;
    auto gap = 0;

    speed_text_->SetPosition(view_rect.left_ + half_w - width / 2.0, top);

    top += height + gap;
    width = speed_hint_text_->GetRowWidth(0);
    speed_hint_text_->SetPosition(view_rect.left_ + half_w - width / 2.0, top);

    width = status_text_->GetRowWidth(0);
    height = status_text_->GetRowHeight() * status_text_->GetNumRows();
    int bottom_gap = 20;
    status_text_->SetPosition(view_rect.left_ + half_w - width / 2.0, view_rect.bottom_ - height - bottom_gap);

    if (car_status_.turn_signal == 0)
    {
        turn_signal_time_ = 0;
        left_turn_signal_sprite_->SetVisible(false);
        right_turn_signal_sprite_->SetVisible(false);
    }
    else if (car_status_.turn_signal == 1)
    {
        turn_signal_time_ += dt;
        if (turn_signal_time_ >= turn_signal_flash_time)
        {
            turn_signal_time_ = 0.0;
            left_turn_signal_sprite_->SetVisible(!left_turn_signal_sprite_->IsVisible());
        }
        right_turn_signal_sprite_->SetVisible(false);
    }
    else if (car_status_.turn_signal == 2)
    {
        turn_signal_time_ += dt;
        if (turn_signal_time_ >= turn_signal_flash_time)
        {
            turn_signal_time_ = 0.0;
            right_turn_signal_sprite_->SetVisible(!right_turn_signal_sprite_->IsVisible());
        }
        left_turn_signal_sprite_->SetVisible(false);
    }
    else
    {
        turn_signal_time_ += dt;
        if (turn_signal_time_ >= turn_signal_flash_time)
        {
            turn_signal_time_ = 0.0;
            left_turn_signal_sprite_->SetVisible(!left_turn_signal_sprite_->IsVisible());
            right_turn_signal_sprite_->SetVisible(left_turn_signal_sprite_->IsVisible());
        }
    }

    ad_on_sprite_->SetVisible(car_status_.ad_on);
    ad_off_sprite_->SetVisible(!car_status_.ad_on);
    ad_on_sprite_->SetRotation(-car_status_.steering_wheel);
    ad_off_sprite_->SetRotation(-car_status_.steering_wheel);

    auto left = 30;
    top = vh / 2.0F;
    gap = 30;
    debug_setting_btn_->SetPosition(left, top);
    debug_log_btn_->SetPosition(left + debug_setting_btn_->GetRowWidth(0) + gap, top);
    debug_clean_data_btn_->SetPosition(left, top + debug_setting_btn_->GetRowHeight() + gap);
    debug_no_log_btn_->SetPosition(left + debug_clean_data_btn_->GetRowWidth(0) + gap,
                                   top + debug_setting_btn_->GetRowHeight() + gap);

    gear_d_text_->SetTextEffect(TE_NONE);
    gear_p_text_->SetTextEffect(TE_NONE);
    gear_r_text_->SetTextEffect(TE_NONE);
    gear_n_text_->SetTextEffect(TE_NONE);
    gear_d_text_->SetColor(gear_color);
    gear_p_text_->SetColor(gear_color);
    gear_r_text_->SetColor(gear_color);
    gear_n_text_->SetColor(gear_color);

    auto* focus_text = gear_d_text_;
    if (car_status_.gear == GEAR_P)
    {
        focus_text = gear_p_text_;
    }
    else if (car_status_.gear == GEAR_D)
    {
        focus_text = gear_d_text_;
    }
    else if (car_status_.gear == GEAR_N)
    {
        focus_text = gear_n_text_;
    }
    else if (car_status_.gear == GEAR_R)
    {
        focus_text = gear_r_text_;
    }

    focus_text->SetTextEffect(TE_STROKE);
    focus_text->SetColor(Color(0.7, 0.7, 0.7));

    int gear_w_offset = 10;
    int gear_h_offset = 10;
    auto gear_h = gear_p_text_->GetRowHeight();
    auto gear_w = gear_p_text_->GetRowWidth(0);
    gap = 10;
    auto gear_x = gear_w_offset;
    auto gear_y = vh - gear_h - gear_h_offset;
    gear_p_text_->SetPosition(gear_x, gear_y);
    gear_x += (gap + gear_w);
    gear_r_text_->SetPosition(gear_x, gear_y);
    gear_x += (gap + gear_w);
    gear_n_text_->SetPosition(gear_x, gear_y);
    gear_x += (gap + gear_w);
    gear_d_text_->SetPosition(gear_x, gear_y);
}

void Game::UpdateKeyInput(float dt)
{
    if (input_->GetKeyPress(KEY_1))
    {
        config_.debug = !config_.debug;
        Sample::InitMouseMode(config_.debug ? MM_FREE : MM_RELATIVE);
    }
    else if (input_->GetKeyPress(KEY_2))
    {
        int input_options[] = {GEAR_P, GEAR_D, GEAR_N, GEAR_R};
        static int index = 0;
        car_status_.gear = input_options[index];
        index++;
        index %= 4;
    }
    else if (input_->GetKeyPress(KEY_3))
    {
        target_pitch_ = config_.camera_init_pitch / 2;
    }
    else if (input_->GetKeyPress(KEY_4))
    {
        target_pitch_ = config_.camera_init_pitch * 2;
    }
    else if (input_->GetKeyPress(KEY_5))
    {
        target_dist_ = config_.camera_init_dist / 2;
    }
    else if (input_->GetKeyPress(KEY_6))
    {
        target_dist_ = config_.camera_init_dist * 2;
    }
    else if (input_->GetKeyPress(KEY_7))
    {
        car_status_.ad_on = !car_status_.ad_on;
    }
    else if (input_->GetKeyPress(KEY_8))
    {
        car_status_.turn_signal++;
        car_status_.turn_signal = car_status_.turn_signal % 4;
    }
    else if (input_->GetKeyPress(KEY_9))
    {
        car_status_.brake_lights = !car_status_.brake_lights;
    }
    else if (input_->GetKeyPress(KEY_0))
    {
        op_status_++;
        if (op_status_ > 2)
            op_status_ = -1;
        car_status_.speed_kmh = 1;
        message_time_ = time_->GetElapsedTime();
    }
}

void Game::HandleTouchEnd(StringHash eventType, VariantMap& eventData)
{
    using namespace TouchEnd;
    auto x = eventData[P_X].GetInt();
    auto y = eventData[P_Y].GetInt();

    auto* uiRoot = GetSubsystem< UI >();
    auto touch_element = uiRoot->GetElementAt(x, y, false);
    if (!touch_element)
        return;

    OnUIClicked(touch_element);
}

void Game::HandleControlClicked(StringHash eventType, VariantMap& eventData)
{
    auto* clicked = static_cast< UIElement* >(eventData[UIMouseClick::P_ELEMENT].GetPtr());
    OnUIClicked(clicked);
}

void Game::HandleMouseButtonUp(StringHash eventType, VariantMap& eventData)
{
    OnUIClicked(ui_->GetElementAt(input_->GetMousePosition(), false));
}

void Game::OnUIClicked(UIElement* e)
{
    if (!e)
        return;

    URHO3D_LOGINFOF("OnUIClicked ui=%s", e->GetName().CString());
    if (e == debug_setting_btn_)
    {
        if (op_debug_mode_ == 0)
            op_debug_mode_ = 1;
        else if (op_debug_mode_ == 1)
            op_debug_mode_ = 0;
    }
    else if (e == debug_log_btn_)
    {
    }
    else if (e == debug_no_log_btn_)
    {
        op_debug_mode_ = 2;
    }
    else if (e == debug_clean_data_btn_)
    {
    }
}

void Game::HandleCustomMessage(SharedPtr< JSONFile > json)
{
    op_status_ = 1;
    message_time_ = time_->GetElapsedTime();
    const auto& json_root = json->GetRoot();
    car_status_.speed_kmh = json_root.Get("speed").GetFloat() * 3.6F;
    car_status_.gear = json_root.Get("gear").GetInt();
    car_status_.steering_wheel = json_root.Get("steering_wheel").GetFloat();
    car_status_.pos_x = json_root.Get("x").GetFloat();
    car_status_.pos_y = json_root.Get("y").GetFloat();
    car_status_.turn_signal = json_root.Get("turn_signal").GetInt();
    car_status_.brake_lights = json_root.Get("brake_lights").GetBool();
    car_status_.ad_on = json_root.Get("ad_on").GetBool();

    car_status_.slots.clear();

    const auto& json_slots = json_root.Get("slot_detection");
    for (auto i = 0; i < json_slots.Size(); ++i)
    {
        const auto& json_slot = json_slots[i];
        Slot slot;
        int j = 0;
        slot.points.Push(Vector3(-json_slot[j + 1].GetFloat(), 0, json_slot[j].GetFloat()));
        j += 2;
        slot.points.Push(Vector3(-json_slot[j + 1].GetFloat(), 0, json_slot[j].GetFloat()));
        j += 2;
        slot.points.Push(Vector3(-json_slot[j + 1].GetFloat(), 0, json_slot[j].GetFloat()));
        j += 2;
        slot.points.Push(Vector3(-json_slot[j + 1].GetFloat(), 0, json_slot[j].GetFloat()));
        car_status_.slots.push_back(slot);
    }
}
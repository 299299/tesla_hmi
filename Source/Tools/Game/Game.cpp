
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

static const int GPS = 1;
static const int EMULATOR = 2;
static const int CRUISE = 3;

static const int SET_IP = 2;
static const int GAME_INIT = 3;
static const int NAVI_INIT = 4;
static const int NAVI_ROUTE_NOTIFY = 5;
static const int NAVI_ARRIVED = 6;
static const int NAVI_TEXT = 7;
static const int NAVI_INFO = 8;
static const int NAVI_MAP_TYPE = 9;
static const int NAVI_CAMERA_INFO = 10;
static const int NAVI_INFO2 = 11;
static const int NAVI_FACILITY = 12;

// doc https://a.amap.com/lbs/static/unzip/Android_Navi_Doc/index.html
static const int NAVI_ICON_LEFT = 2;
static const int NAVI_ICON_RIGHT = 3;
static const int NAVI_ICON_STRAIGHT = 9;
static const int NAVI_ICON_RIGHT_BACK = 7;
static const int NAVI_ICON_RIGHT_FRONT = 5;
static const int NAVI_ICON_LEFT_BACK = 6;
static const int NAVI_ICON_LEFT_FRONT = 4;

static const int NAVI_CAMERA_SPEED_LIMIT = 0;
static const int NAVI_CAMERA_BREAKRULE = 3;
static const int NAVI_CAMERA_BUS_WAY = 4;
static const int NAVI_CAMERA_EMERGENCY = 5;
static const int NAVI_CAMERA_SURVEILLANCE = 1;
static const int NAVI_CAMERA_TRAFFICLIGHT = 2;

static const int FROM_NATIVE_GPS = 100;

const float turn_signal_flash_time = 0.5F;

const float RADAR_FILTER_D_DIST = 6.0F;
const float LANE_CHANGE_LINE_PROB = 0.5F;

const float LC_ANIM_TIMER = 6.0F;
const float LC_ANIM_STEP_TIMER = 0.5F;

enum CameraState
{
    kCameraFPS,
    kCameraTP,
    kCameraFixed,
};

// enum cereal_CarState_GearShifter {
//     cereal_CarState_GearShifter_unknown = 0,
//     cereal_CarState_GearShifter_park = 1,
//     cereal_CarState_GearShifter_drive = 2,
//     cereal_CarState_GearShifter_neutral = 3,
//     cereal_CarState_GearShifter_reverse = 4,
//     cereal_CarState_GearShifter_sport = 5,
//     cereal_CarState_GearShifter_low = 6,
//     cereal_CarState_GearShifter_brake = 7,
//     cereal_CarState_GearShifter_eco = 8,
//     cereal_CarState_GearShifter_manumatic = 9
// };

Color gear_color(0.4, 0.4, 0.4);

URHO3D_DEFINE_APPLICATION_MAIN(Game);

#ifdef __ANDROID__
extern "C" {
extern int Android_JNI_SendMessage2(int cmd, double data1, double data2, double data3, double data4, double data5);
}
#endif

static void read_points(cereal_ModelDataV2_XYZTData& xyzt_data, PODVector< Vector3 >& ret)
{
    auto& x_list = xyzt_data.x;
    auto& y_list = xyzt_data.y;

    auto len = capn_len(x_list);

    // printf("read_points len=%d \n", len);

    capn_resolve(&x_list.p);
    capn_resolve(&y_list.p);

    ret.Resize(len);

    for (int i = 0; i < len; ++i)
    {
        auto x = capn_to_f32(capn_get32(x_list, i));
        auto y = capn_to_f32(capn_get32(y_list, i));

        // ret[i] = Vector3(-y, 0.0F, x);
        ret[i] = Vector3(y, 0.0F, x);

        // printf("pos=%s\n", ret[i].ToString().CString());
    }
}

static void read_points(cereal_ModelDataV2_XYZTData_ptr xyztp, PODVector< Vector3 >& ret)
{
    cereal_ModelDataV2_XYZTData xyzt_data;
    cereal_read_ModelDataV2_XYZTData(&xyzt_data, xyztp);
    read_points(xyzt_data, ret);
}

static void visualize_lane_geometry(const PODVector< Vector3 >& pvd,
                                    CustomGeometry* lane_geometry,
                                    Material* mat,
                                    float width,
                                    bool solid,
                                    float max_distance,
                                    float x_offset = 0.0F)
{
    lane_geometry->Clear();
    lane_geometry->SetNumGeometries(1);
    lane_geometry->SetDynamic(true);

    Vector3 offset(x_offset, 0, 0);

    if (solid)
    {
        lane_geometry->BeginGeometry(0, TRIANGLE_STRIP);
        for (const auto& pos : pvd)
        {
            if (pos.z_ > max_distance)
                break;
            lane_geometry->DefineVertex(pos + Vector3(width, 0, 0) + offset);
            lane_geometry->DefineVertex(pos + Vector3(-width, 0, 0) + offset);
        }
    }
    else
    {
        lane_geometry->BeginGeometry(0, TRIANGLE_LIST);
        const int N = 10;
        int cut_num = N;

        for (int i = 0; i < pvd.Size() - 1; ++i)
        {
            int k = i / N;
            if (k % 2 != 0)
                continue;

            auto pos0 = pvd[i];
            auto pos1 = pvd[i + 1];
            auto p0 = pos0 + Vector3(width, 0, 0);
            auto p1 = pos0 + Vector3(-width, 0, 0);
            auto p2 = pos1 + Vector3(width, 0, 0);
            auto p3 = pos1 + Vector3(-width, 0, 0);

            if (p0.z_ > max_distance)
                break;

            lane_geometry->DefineVertex(p0 + offset);
            lane_geometry->DefineVertex(p1 + offset);
            lane_geometry->DefineVertex(p3 + offset);
            lane_geometry->DefineVertex(p3 + offset);
            lane_geometry->DefineVertex(p2 + offset);
            lane_geometry->DefineVertex(p0 + offset);
        }
    }

    lane_geometry->SetMaterial(mat);
    lane_geometry->Commit();
}

static void visualize_line(LineVisData& data, float max_distance)
{
    visualize_lane_geometry(data.points, data.geometry, data.material, data.width, data.solid, max_distance);
}

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
      model_changed_(false),
      message_time_(0),
      speed_(0),
      longitude_(0),
      latitude_(0),
      touch_down_time_(0),
      debug_touch_flag_(0),
      android_(false),
      num_cpu_cores_(0),
      turn_signal_time_(0),
      turn_signal_(0),
      last_speed_limit_(0),
      target_pitch_(-999.0F),
      target_dist_(-1.0F),
      target_yaw_(-999.0F),
      control_enabled_(false),
      touch_up_time_(0.0F),
      camera_state_(kCameraTP),
      show_live_tracks_(true),
      op_debug_mode_(0),
      set_speed_(0.0F),
      steering_wheel_(0.0F),
      speed_ms_(0),
      lc_dir_(0),
      lc_send_frames_(0),
      brake_lights_(false),
      road_edge_left_(10.0),
      road_edge_right_(10.0),
      car_gear_(1),
      camera_blend_speed_(0.1F),
      camera_blend_acceleration_(0.1F),
      lc_state_(0),
      status_text_time_out_(5.0F),
      debug_test_(false),
      lc_navigation_(0)
{
    URHO3D_LOGINFOF("************************ Game::Game() tid=%u ************************ ", pthread_self());

    memset(&model_, 0x00, sizeof(model_));
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
        const char* ip_list[] = {"192.168.3.9", "192.168.43.138", "192.168.137.138"};
#else
        const char* ip_list[] = {"192.168.5.11", "192.168.3.138", "192.168.43.138", "192.168.137.138", "127.0.0.1"};
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
        "modelV2",
        // "liveCalibration",
        // "gpsLocationExternal",
        "controlsState",
        // "thumbnail",
        // "dMonitoringState",
        "carState",
        "liveTracks",
        // "pathPlan",
        "radarState",
        // "pathPlan",
        // "laneSpeed",
        "testJoystick",
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

    sync_pub_ = OP::PubSocket::create(op_ctx_, "testLiveLocation");
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
    // SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Game, HandleUpdate));
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
    {
        MutexLock _l(lock_);
        nav_data_main_thread_ = nav_data_java_thread_;
    }

    ReceiveDataFromOP();
    SyncUI(timeStep);
    UpdateInput(timeStep);
    UpdateNavigation(timeStep);
    SyncToOP();
}

void Game::ReceiveDataFromOP()
{
    auto cur_time = time_->GetElapsedTime();
    if (cur_time - message_time_ > 5.0 && op_status_ == 1)
    {
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

        if (sock == socks_.back())
        {
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
        }
        else
        {
            HandleOPMessage(msg);
        }

        delete msg;
    }
}

void Game::SyncToOP()
{
    auto elapsed = time_->GetElapsedTime();

    if (elapsed - last_sync_time_ > 0.025F)
    {
        sync_str_ = "{";
        sync_str_ += "\"speed_limit\":" + String(nav_data_main_thread_.speed_limit) + ",";
        sync_str_ += "\"navi_icon\":" + String(nav_data_main_thread_.navi_icon) + ",";
        sync_str_ += "\"dist_to_next_step\":" + String(nav_data_main_thread_.dist_to_next_step) + ",";
        sync_str_ += "\"has_exit\":" + String(!nav_data_main_thread_.exit_name.Empty()) + ",";
        sync_str_ += "\"navi_type\":" + String(nav_data_main_thread_.navi_type) + ",";
        sync_str_ += "\"remain_dist\":" + String(nav_data_main_thread_.remain_dist) + ",";
        sync_str_ += "\"is_high_way\":" + String(nav_data_main_thread_.high_way) + ",";
        sync_str_ += "\"op_debug_mode\":" + String(op_debug_mode_) + ",";
        sync_str_ += "\"date\":\"" + Time::GetTimeStamp() + "\"" + ",";
        sync_str_ += "\"cmd_line\":\"" + op_cmd_line_ + "\"" + ",";
        sync_str_ += "\"lc_dir\":\"" + String(lc_dir_) + "\"";

        if (lc_send_frames_ > 0)
        {
            lc_send_frames_--;
        }

        if (lc_send_frames_ <= 0)
        {
            lc_dir_ = 0;
            lc_send_frames_ = 0;
        }

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

    lead_car_ = CreateCarModel("LeadCar", "MY/LeadCar.xml");
    lead_car2_ = CreateCarModel("LeadCar2", "MY/LeadCar2.xml");

    for (int i = 0; i < MAX_LIVE_TRACKS; ++i)
    {
        track_nodes_[i] = CreateCarModel("LeadCar", "MY/Track.xml");
    }

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

    auto* base_mat = cache_->GetResource< Material >("MY/Lane.xml");
    Vector3 vis_position(0, 0, CAR_LENGTH - RADAR_TO_CAMERA);

    path_vis_.node = scene_->CreateChild("Path");
    path_vis_.node->SetPosition(vis_position);
    path_vis_.geometry = path_vis_.node->CreateComponent< CustomGeometry >();
    path_vis_.material = base_mat->Clone();
    path_vis_.width = config_.path_line_width;

    for (int i = 0; i < 2; ++i)
    {
        auto& road_edge = road_edge_vis_[i];
        road_edge.node = scene_->CreateChild("RoadEdge_" + Urho3D::String(i));
        road_edge.node->SetPosition(vis_position);
        road_edge.geometry = road_edge.node->CreateComponent< CustomGeometry >();
        road_edge.material = base_mat->Clone();
        road_edge.width = config_.road_edge_width;
    }

    for (int i = 0; i < 4; ++i)
    {
        auto& lane = lane_vis_[i];
        lane.node = scene_->CreateChild("Lane_" + Urho3D::String(i));
        lane.node->SetPosition(vis_position);
        lane.geometry = lane.node->CreateComponent< CustomGeometry >();
        lane.material = base_mat->Clone();
        lane.width = config_.lane_line_width;
    }

    UpdateLaneColor();
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

    if (car_gear_ == 1 || car_gear_ == 4 || car_gear_ == 3)
        camera_state_ = kCameraFixed;
    else if (car_gear_ == 2)
        camera_state_ = kCameraTP;

    if (camera_state_ == kCameraFPS)
        UpdateFPSCamera(timeStep);
    else if (camera_state_ == kCameraTP)
        UpdateTPCamera(timeStep);
    else
        UpdateFixedCamera(timeStep);

    UpdateKeyInput(timeStep);

    bool lane_line_visible = camera_state_ != kCameraFixed;
    for (auto& l : lane_vis_)
        l.node->SetEnabled(lane_line_visible);
    path_vis_.node->SetEnabled(lane_line_visible);
    for (auto& l : road_edge_vis_)
        l.node->SetEnabled(lane_line_visible);
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
    debug_text_->SetFont(font, android_ ? config_.debug_text_size : 10);
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

    set_speed_text_ = uiRoot->CreateChild< Text >("set_speed");
    set_speed_text_->SetFont(font, config_.speed_text_size / 4.0);
    set_speed_text_->SetColor(SOFT_PURPLE);
    set_speed_text_->SetPriority(-10);
    ui_elements_.Push(set_speed_text_);

    status_text_ = uiRoot->CreateChild< Text >("status");
    status_text_->SetTextEffect(TE_SHADOW);
    status_text_->SetFont(font, config_.status_text_size);
    status_text_->SetColor(Color(26 / 255.0, 80 / 255.0, 139 / 255.0));
    status_text_->SetText("OP CONNECTING");
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
        speed_limit_sprite_ = uiRoot->CreateChild< Sprite >("speed_limit_sprite");
        speed_limit_sprite_->SetVisible(false);
        speed_limit_sprite_->SetOpacity(0.9f);
        speed_limit_sprite_->SetTexture(cache_->GetResource< Texture2D >("MY/traffic_sign_speed_lim_10.png"));
        speed_limit_sprite_->SetSize(config_.speed_limit_size, config_.speed_limit_size);
        speed_limit_sprite_->SetHotSpot(config_.speed_limit_size / 2.0, config_.speed_limit_size / 2.0);
        speed_limit_sprite_->SetPosition(
            vw - config_.speed_limit_size - config_.right_gap + config_.speed_limit_size / 2.0, icon_top);
        ui_elements_.Push(speed_limit_sprite_);
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

    auto lc_btn_size = config_.icon_size * 0.75F;
    left_lc_btn_ = uiRoot->CreateChild< Sprite >("left_lc");
    left_lc_btn_->SetTexture(cache_->GetResource< Texture2D >("MY/left_lc.png"));
    left_lc_btn_->SetSize(lc_btn_size, lc_btn_size);
    left_lc_btn_->SetHotSpot(lc_btn_size / 2.0, lc_btn_size / 2.0);
    // left_lc_btn_->SetOpacity(0.9f);
    int lc_w_offset = 10;
    int lc_h_offset = 10;
    auto lc_w = lc_btn_size;
    auto lc_h = lc_btn_size;
    left_lc_btn_->SetPosition(vw - 2 * lc_w - gap - lc_w_offset, vh - lc_h - lc_h_offset);
    ui_elements_.Push(left_lc_btn_);

    right_lc_btn_ = uiRoot->CreateChild< Sprite >("right_lc");
    right_lc_btn_->SetTexture(cache_->GetResource< Texture2D >("MY/right_lc.png"));
    right_lc_btn_->SetSize(lc_btn_size, lc_btn_size);
    right_lc_btn_->SetHotSpot(lc_btn_size / 2.0, lc_btn_size / 2.0);
    right_lc_btn_->SetVisible(true);
    right_lc_btn_->SetPosition(vw - lc_w - lc_w_offset, vh - lc_h - lc_h_offset);
    ui_elements_.Push(right_lc_btn_);

    gear_p_text_ = uiRoot->CreateChild< Text >("P");
    gear_p_text_->SetFont(font, config_.debug_ui_size);
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
    gear_r_text_->SetFont(font, config_.debug_ui_size);
    gear_r_text_->SetText("R");
    gear_r_text_->SetColor(gear_color);
    gear_r_text_->SetPosition(gear_x, gear_y);
    ui_elements_.Push(gear_r_text_);

    gear_x += (gap + gear_w);
    gear_n_text_ = uiRoot->CreateChild< Text >("N");
    gear_n_text_->SetFont(font, config_.debug_ui_size);
    gear_n_text_->SetText("N");
    gear_n_text_->SetPosition(gear_x, gear_y);
    gear_n_text_->SetColor(gear_color);
    ui_elements_.Push(gear_n_text_);

    gear_x += (gap + gear_w);
    gear_d_text_ = uiRoot->CreateChild< Text >("D");
    gear_d_text_->SetFont(font, config_.debug_ui_size);
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

void Game::UpdateLaneColor()
{
    // float alpha1 = std::min(model_.left_lane.prob * 3.0F, 1.0F);
    // alpha1 = std::max(alpha1, 0.5F);
    if (control_enabled_)
    {
        for (int i = 0; i < 4; ++i)
        {
            auto mat = lane_vis_[i].material;
            auto alpha = model_.lanes[i].prob;
            mat->SetShaderParameter("MatDiffColor", Vector4(AD_ON_COLOR.r_, AD_ON_COLOR.g_, AD_ON_COLOR.b_, alpha));
        }
    }
    else
    {
        for (int i = 0; i < 4; ++i)
        {
            auto mat = lane_vis_[i].material;
            auto alpha = model_.lanes[i].prob;
            // alpha = 1.0F;
            mat->SetShaderParameter(
                "MatDiffColor", Vector4(config_.lane_color[0], config_.lane_color[1], config_.lane_color[2], alpha));
        }
    }

    path_vis_.material->SetShaderParameter(
        "MatDiffColor",
        Vector4(config_.path_color[0], config_.path_color[1], config_.path_color[2], config_.path_color[3]));

    for (int i = 0; i < 2; ++i)
    {
        // printf("edge %d std=%f \n", i, model_.edges[i].std);
        auto edge_apha = Clamp(1.0F - model_.edges[i].std, 0.0F, 1.0F);
        road_edge_vis_[i].material->SetShaderParameter(
            "MatDiffColor", Vector4(config_.edge_color[0], config_.edge_color[1], config_.edge_color[2], edge_apha));
    }
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

    if (touch_up_time_ > config_.camera_reset_time && speed_ > 0.0)
    {
        target_pitch_ = config_.camera_init_pitch;
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
            else if (debug_touch_flag_ == 2)
            {
                // top right
                // config_.thumbnail = !config_.thumbnail;
                // show_live_tracks_ = !show_live_tracks_;
                int input_options[] = {1, 2, 3, 4};
                static int index = 0;
                car_gear_ = input_options[index];
                index++;
                index %= 4;
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
                turn_signal_++;
                turn_signal_ = turn_signal_ % 4;
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
    MutexLock _m(lock_);
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
        case NAVI_INIT:
            URHO3D_LOGINFOF("navi init %f %f %f %s ", data1, data2, data3, data4);
            nav_data_java_thread_.navi_type = (int)data1;
            break;
        case NAVI_ROUTE_NOTIFY:
            break;
        case NAVI_INFO:
        {
            URHO3D_LOGINFOF(
                "NAVI_INFO dist_to_next_step=%d time=%d icon=%d exit=%s ", (int)data1, (int)data2, (int)data3, data4);
            nav_data_java_thread_.exit_name = data4;
            nav_data_java_thread_.navi_icon = (int)data3;
            nav_data_java_thread_.dist_to_next_step = (int)data1;
            break;
        }
        case NAVI_INFO2:
        {
            URHO3D_LOGINFOF("NAVI_INFO2 dist_remain=%d time=%d step=%d cur_road_name=%s ",
                            (int)data1,
                            (int)data2,
                            (int)data3,
                            data4);
            nav_data_java_thread_.remain_dist = (int)data1;
            nav_data_java_thread_.cur_road_name = data4;
            auto& cur_road_name = nav_data_java_thread_.cur_road_name;

            bool is_high_way = cur_road_name.EndsWith("高速");
            bool is_elevated_road = cur_road_name.EndsWith("高架");
            nav_data_java_thread_.high_way = is_high_way || is_elevated_road;

            // this is pretty hard to do such kind of configuration
            // but I think we put the configuration in phone site might be more suitable
            // in the future we can put this kind of config in to a website as json config
            if (nav_data_java_thread_.high_way)
            {
                if (cur_road_name.StartsWith("G2"))
                {
                    nav_data_java_thread_.speed_limit = 120;
                }
                else
                {
                    nav_data_java_thread_.speed_limit = 100;
                }
            }
            else if (is_elevated_road)
            {
                nav_data_java_thread_.speed_limit = 80;
            }
            else
            {
                nav_data_java_thread_.speed_limit = 60;
            }

            break;
        }
        case NAVI_ARRIVED:
            URHO3D_LOGINFOF("navi arrived %f %f %f %s ", data1, data2, data3, data4);
            break;
        case NAVI_TEXT:
            // nav_msg_ = data4;
            break;
        case NAVI_MAP_TYPE:
            URHO3D_LOGINFOF("navi map type %f %f %f %s ", data1, data2, data3, data4);
            // config_.is_night = ((int)data1) == 3;
            nav_data_java_thread_.map_type = (int)data1;
            break;
        case NAVI_CAMERA_INFO:
            if ((int)data1 == NAVI_CAMERA_SPEED_LIMIT)
            {
                nav_data_java_thread_.speed_limit = (int)data2;
                URHO3D_LOGINFOF(
                    "NAVI_CAMERA_INFO speed_limit camera speed=%d dist=%f ", nav_data_java_thread_.speed_limit, data3);
            }
            break;
        case NAVI_FACILITY:
        {
            int speed_limit = (int)data3;
            if (speed_limit > 0)
            {
                nav_data_java_thread_.speed_limit = speed_limit;
                URHO3D_LOGINFOF(
                    "NAVI_FACILITY speed_limit speed=%d dist=%f ", nav_data_java_thread_.speed_limit, data1);
            }
        }
        break;
    }
}

void Game::HandleOPMessage(OP::Message* msg)
{
    op_status_ = 1;
    message_time_ = time_->GetElapsedTime();

    struct capn ctx;
    capn_init_mem(&ctx, (uint8_t*)msg->getData(), msg->getSize(), 0);

    cereal_Event_ptr eventp;
    eventp.p = capn_getp(capn_root(&ctx), 0, 1);
    struct cereal_Event eventd;
    cereal_read_Event(&eventd, eventp);

    // URHO3D_LOGINFOF("op msg id = %d", eventd.which);

    if (eventd.which == cereal_Event_modelV2)
    {
        HandleOPModel(eventd);
    }
    else if (eventd.which == cereal_Event_carState)
    {
        HandleOPCarState(eventd);
    }
    else if (eventd.which == cereal_Event_liveTracks)
    {
        HandleOPLiveTracks(eventd);
    }
    else if (eventd.which == cereal_Event_radarState)
    {
        HandleOPRadarState(eventd);
    }
    else if (eventd.which == cereal_Event_controlsState)
    {
        HandleOPControlState(eventd);
    }

    capn_free(&ctx);
}

void Game::HandleCustomMessage(SharedPtr< JSONFile > json)
{
    const auto& json_root = json->GetRoot();
    // op_custom_.left_lane = json_root.Get("left_lane").GetBool();
    // op_custom_.right_lane = json_root.Get("right_lane").GetBool();
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
    Urho3D::String info_text = " camera-dist: " + String(camera_dist_) + " pitch=" + String(pitch_) + " cpu-core=" +
                               String(num_cpu_cores_) + " \n";
    char buf[256] = {0};
    Urho3D::String text;
    if (op_status_ == 1)
    {
        for (int i = 0; i < 2; ++i)
        {
            const auto& lead = model_.leads[i];
            snprintf(buf,
                     sizeof(buf),
                     "\n modeld lead %d  status=%d dRel=%.2f, yRel=%.2f, vLead=%.2f",
                     i,
                     lead.status,
                     lead.dRel,
                     lead.yRel,
                     lead.vLead);
            text += String(buf);
        }

        snprintf(buf,
                 sizeof(buf),
                 "\n edge_0_std=%.2f  edge_1_std=%.2f line_0_prob=%.2f, line_1_prob=%.2f, line_2_prob=%.2f, "
                 "line_3_prob=%.2f ",
                 model_.edges[0].std,
                 model_.edges[1].std,
                 model_.lanes[0].prob,
                 model_.lanes[1].prob,
                 model_.lanes[2].prob,
                 model_.lanes[3].prob);
        text += String(buf);
    }
    else if (op_status_ == 2)
    {
        info_text += String(" OP message time out !!!");
    }
    else if (op_status_ == 0)
    {
        info_text += String(" waiting for connection of OP");
    }
    else if (op_status_ == -1)
    {
        info_text += op_ip_address_ + String(" ping failed");
    }

    info_text += text;
    info_text += "\n speed_limit=" + String(nav_data_main_thread_.speed_limit) + " icon=" +
                 String(nav_data_main_thread_.navi_icon) + " exit_name=" + nav_data_main_thread_.exit_name;
    info_text += "\n longitude=" + String(longitude_) + " latitude=" + String(latitude_);
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

        for (int i = 0; i < 2; ++i)
        {
            for (auto p : road_edge_vis_[i].points)
            {
                Sphere sp(p, 0.1F);
                debug->AddSphere(sp, Color::RED, false);
            }
        }

        for (int i = 0; i < 4; ++i)
        {
            for (auto p : lane_vis_[i].points)
            {
                Sphere sp(p, 0.1F);
                debug->AddSphere(sp, Color::BLUE, false);
            }
        }

        for (auto p : path_vis_.points)
        {
            Sphere sp(p, 0.1F);
            debug->AddSphere(sp, Color::YELLOW, false);
        }
    }

    // auto* cam = cameraNode_->GetComponent< Camera >();
    // cam->SetFillMode(config_.debug ? FILL_WIREFRAME : FILL_SOLID);
}

void Game::Draw3D(float dt)
{
    const auto& lead1 = model_.leads[0];
    const auto& lead2 = model_.leads[1];

    tail_light_->SetEnabled(brake_lights_);

    lead_car_->SetEnabled(lead1.status && camera_state_ != kCameraFixed);
    float dist = lead1.dRel - RADAR_TO_CAMERA + CAR_LENGTH;
    lead_car_->SetPosition(Vector3(-lead1.yRel, 0, dist));

    lead_car2_->SetEnabled(lead2.status && camera_state_ != kCameraFixed);
    dist = lead2.dRel - RADAR_TO_CAMERA + CAR_LENGTH;
    lead_car2_->SetPosition(Vector3(-lead2.yRel, 0, dist));

    if (model_changed_)
    {
        float lead_d = lead1.dRel;
        float path_length = (lead_d > 0.0F) ? lead_d - Min(lead_d * 0.35F, 10.0F) : MAX_DRAW_DISTANCE;
        path_length = fmin(path_length, model_.max_distance);

        visualize_line(path_vis_, path_length);
        for (int i = 0; i < 2; ++i)
            visualize_line(road_edge_vis_[i], model_.max_distance);
        for (int i = 0; i < 4; ++i)
            visualize_line(lane_vis_[i], model_.max_distance);

        UpdateLaneColor();

        model_changed_ = false;
    }

    for (int i = 0; i < MAX_LIVE_TRACKS; ++i)
        track_nodes_[i]->SetEnabled(false);

    if (show_live_tracks_)
    {
        auto lead_pos = lead_car_->GetPosition();
        auto lead2_pos = lead_car2_->GetPosition();
        const auto dist_x_threshold = 2.0F;
        const auto dist_z_threshold = 5.0F;

        HashMap< int, bool > pos_map;
        int pos_candidates[64];

        const int x_gap = 2;
        const int z_gap = 3;

        for (int i = 0; i < live_tracks_.Size(); ++i)
        {
            auto pos = track_nodes_[i]->GetPosition();
            if (lead1.status)
            {
                auto diff_x = Abs(pos.x_ - lead_pos.x_);
                auto diff_z = Abs(pos.z_ - lead_pos.z_);
                if (diff_x < dist_x_threshold && diff_z < dist_z_threshold)
                    continue;
            }
            if (lead2.status)
            {
                auto diff_x = Abs(pos.x_ - lead2_pos.x_);
                auto diff_z = Abs(pos.z_ - lead2_pos.z_);
                if (diff_x < dist_x_threshold && diff_z < dist_z_threshold)
                    continue;
            }

            int x_i = int(pos.x_);
            int z_i = int(pos.z_);
            int index = 0;

            for (int i = -x_gap; i <= x_gap; ++i)
            {
                for (int j = -z_gap; j <= z_gap; ++j)
                {
                    pos_candidates[index++] = (x_i + i) * 1000 + (z_i + j);
                }
            }

            bool pos_repeated = false;
            for (int i = 0; i < index; ++i)
            {
                if (pos_map.Contains(pos_candidates[i]))
                {
                    pos_repeated = true;
                    break;
                }

                pos_map[pos_candidates[i]] = true;
            }

            if (pos_repeated)
                continue;

            track_nodes_[i]->SetEnabled(true);
        }
    }
}

void Game::Draw2D(float dt)
{
    bool lead_visible = lead_car_->IsEnabled();
    bool lead2_visible = lead_car2_->IsEnabled();

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
        String text = String((int)speed_);
        speed_text_->SetText(text);
        int v = set_speed_;
        if (v == 255)
            v = 0;
        set_speed_text_->SetText(String(v));
        speed_hint_text_->SetText("km/h");
    }
    else if (op_status_ == 2)
    {
        status_text_->SetText("OP message time out !!!      ");
        status_text_time_out_ = 5.0F;
    }
    else if (op_status_ == -1)
    {
        status_text_->SetText(op_ip_address_ + String(" ping failed"));
        status_text_time_out_ = 5.0F;
    }
    else if (op_status_ == 0)
    {
        status_text_->SetText("OP CONNECTING");
        status_text_time_out_ = 5.0F;
    }

    if (!nav_data_main_thread_.exit_name.Empty())
    {
        status_text_->SetText(nav_data_main_thread_.exit_name);
    }

    auto width = speed_text_->GetRowWidth(0);
    auto height = speed_text_->GetRowHeight();
    auto top = 10;
    auto gap = 0;

    speed_text_->SetPosition(view_rect.left_ + half_w - width / 2.0, top);

    // top += height + gap;
    auto set_speed_width = set_speed_text_->GetRowWidth(0);
    auto set_speed_height = set_speed_text_->GetRowHeight();
    set_speed_text_->SetPosition(view_rect.left_ + half_w - set_speed_width / 2.0 + width,
                                 top + height / 2.0 - set_speed_height / 2.0);

    top += height + gap;
    width = speed_hint_text_->GetRowWidth(0);
    speed_hint_text_->SetPosition(view_rect.left_ + half_w - width / 2.0, top);

    width = status_text_->GetRowWidth(0);
    height = status_text_->GetRowHeight() * status_text_->GetNumRows();
    int bottom_gap = 20;
    status_text_->SetPosition(view_rect.left_ + half_w - width / 2.0, view_rect.bottom_ - height - bottom_gap);

    if (turn_signal_ == 0)
    {
        turn_signal_time_ = 0;
        left_turn_signal_sprite_->SetVisible(false);
        right_turn_signal_sprite_->SetVisible(false);
    }
    else if (turn_signal_ == 1)
    {
        turn_signal_time_ += dt;
        if (turn_signal_time_ >= turn_signal_flash_time)
        {
            turn_signal_time_ = 0.0;
            left_turn_signal_sprite_->SetVisible(!left_turn_signal_sprite_->IsVisible());
        }
        right_turn_signal_sprite_->SetVisible(false);
    }
    else if (turn_signal_ == 2)
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

    if (last_speed_limit_ != nav_data_main_thread_.speed_limit)
    {
        last_speed_limit_ = nav_data_main_thread_.speed_limit;

        if (last_speed_limit_ > 0)
        {
            auto tex_name = String("MY/traffic_sign_speed_lim_") + String(last_speed_limit_) + ".png";
            auto* tex = cache_->GetResource< Texture2D >(tex_name);
            speed_limit_sprite_->SetTexture(tex);
            speed_limit_sprite_->SetVisible(true);
        }
        else
        {
            speed_limit_sprite_->SetVisible(false);
        }
    }

    bool is_night = (nav_data_main_thread_.map_type == 3);
    if (config_.is_night != is_night)
    {
        config_.is_night = is_night;
        UpdateDayLight();
    }

    ad_on_sprite_->SetVisible(control_enabled_);
    ad_off_sprite_->SetVisible(!control_enabled_);
    ad_on_sprite_->SetRotation(-steering_wheel_);
    ad_off_sprite_->SetRotation(-steering_wheel_);

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
    if (car_gear_ == 1)
    {
        focus_text = gear_p_text_;
    }
    else if (car_gear_ == 2)
    {
        focus_text = gear_d_text_;
    }
    else if (car_gear_ == 3)
    {
        focus_text = gear_n_text_;
    }
    else if (car_gear_ == 4)
    {
        focus_text = gear_r_text_;
    }

    focus_text->SetTextEffect(TE_STROKE);
    focus_text->SetColor(Color(0.7, 0.7, 0.7));

    int lc_w_offset = 10;
    int lc_h_offset = 10;
    auto lc_w = left_lc_btn_->GetSize().x_;
    auto lc_h = left_lc_btn_->GetSize().y_;
    left_lc_btn_->SetPosition(vw - lc_w * 1.5F - gap - lc_w_offset, vh - lc_h / 2 - lc_h_offset);
    right_lc_btn_->SetPosition(vw - lc_w / 2 - lc_w_offset, vh - lc_h / 2 - lc_h_offset);

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

    bool right_visible = (model_.lanes[3].prob >= LANE_CHANGE_LINE_PROB && lc_state_ == 0 && control_enabled_);
    bool left_visible = (model_.lanes[0].prob >= LANE_CHANGE_LINE_PROB && lc_state_ == 0 && control_enabled_);

    if (debug_test_)
    {
        left_visible = true;
        right_visible = true;
    }

    left_lc_btn_->SetVisible(left_visible);
    right_lc_btn_->SetVisible(right_visible);
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
        // config_.thumbnail = !config_.thumbnail;
        // camera_state_++;
        // camera_state_ %= (kCameraFixed + 1);
        // if (camera_state_ == kCameraTP)
        //     camera_state_ = kCameraFixed;
        // else
        //     camera_state_ = kCameraTP;

        int input_options[] = {1, 2, 3, 4};
        static int index = 0;
        car_gear_ = input_options[index];
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
        control_enabled_ = !control_enabled_;
    }
    else if (input_->GetKeyPress(KEY_8))
    {
        turn_signal_++;
        turn_signal_ = turn_signal_ % 4;
    }
    else if (input_->GetKeyPress(KEY_9))
    {
        int speed_options[] = {30, 40, 50, 60, 70, 80, 90, 100, 120};
        static int idx = 0;
        nav_data_java_thread_.speed_limit = speed_options[idx++];
        if (idx >= sizeof(speed_options) / sizeof(speed_options[0]))
            idx = 0;
    }
    else if (input_->GetKeyPress(KEY_0))
    {
        op_status_++;
        if (op_status_ > 2)
            op_status_ = -1;
        message_time_ = time_->GetElapsedTime();
    }
    else if (input_->GetKeyPress(KEY_Q))
    {
        static int _c = 0;
        _c++;
        nav_data_java_thread_.map_type = ((_c % 2) != 0) ? 3 : 1;
    }
    else if (input_->GetKeyPress(KEY_W))
    {
        static bool _b = true;
        model_.lanes[0].prob = _b ? 1.0F : 0.1F;
        model_.lanes[3].prob = _b ? 1.0F : 0.1F;
        _b = !_b;
    }
    else if (input_->GetKeyPress(KEY_E))
    {
        static int _b = 1;
        model_.lanes[0].prob = 1.0F;
        model_.lanes[3].prob = 1.0F;
        _b *= -1;
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
        // op_cmd_line_ = "cd /data/openpilot; git pull; reboot";
        op_debug_mode_ = 3;
    }
    else if (e == debug_no_log_btn_)
    {
        op_debug_mode_ = 2;
    }
    else if (e == debug_clean_data_btn_)
    {
        op_cmd_line_ = "rm -rf /data/media/0/realdata/*";
    }
    else if (e == left_lc_btn_)
    {
        TriggerLaneChange(1);
    }
    else if (e == right_lc_btn_)
    {
        TriggerLaneChange(-1);
    }
}

void Game::HandleOPRadarState(const cereal_Event& eventd)
{
    cereal_RadarState data;
    cereal_read_RadarState(&data, eventd.radarState);

    cereal_RadarState_LeadData lead_data[2];
    cereal_read_RadarState_LeadData(&lead_data[0], data.leadOne);
    cereal_read_RadarState_LeadData(&lead_data[1], data.leadTwo);

    for (int i = 0; i < 2; ++i)
    {
        model_.leads[i].status = lead_data[i].status;
        model_.leads[i].dRel = lead_data[i].dRel;
        model_.leads[i].yRel = lead_data[i].yRel;
        model_.leads[i].vRel = lead_data[i].vRel;
        model_.leads[i].aRel = lead_data[i].aRel;
        model_.leads[i].vLead = lead_data[i].vLead;
    }

    if (Abs(model_.leads[1].yRel - model_.leads[0].yRel) < 3.0)
        model_.leads[1].status = 0;
}

void Game::HandleOPCarState(const cereal_Event& eventd)
{
    struct cereal_CarState data;
    cereal_read_CarState(&data, eventd.carState);

    auto& events = data.events;
    speed_ = data.vEgo * 3.6F;
    speed_ms_ = data.vEgo;
    brake_lights_ = data.brakeLights;

    if (data.leftBlinker && data.rightBlinker)
        turn_signal_ = 3;
    else if (data.leftBlinker)
        turn_signal_ = 1;
    else if (data.rightBlinker)
        turn_signal_ = 2;
    else
        turn_signal_ = 0;

    struct cereal_CarState_CruiseState curiseState;
    cereal_read_CarState_CruiseState(&curiseState, data.cruiseState);

    control_enabled_ = (curiseState.enabled != 0);
    // set_speed_ = curiseState.speed * 3.6F;
    steering_wheel_ = data.steeringAngle;
    car_gear_ = data.gearShifter;

    if (debug_test_)
        control_enabled_ = true;
}

void Game::HandleOPLiveTracks(cereal_Event& eventd)
{
    struct cereal_LiveTracks data;
    auto& tracks = eventd.liveTracks;
    auto len = capn_len(tracks);
    live_tracks_.Clear();
    // live_tracks_.Resize(len);
    // URHO3D_LOGINFOF("live tracks num=%d", len);
    len = Min(len, MAX_LIVE_TRACKS);
    int index = 0;
    for (int i = 0; i < len; ++i)
    {
        struct cereal_LiveTracks track;
        cereal_get_LiveTracks(&track, tracks, i);

        // filter out the radar points which is outside of the road edge
        auto max_y = RADAR_FILTER_D_DIST;
        if (track.yRel < 0)
            max_y = Min(max_y, road_edge_right_);
        else if (track.yRel > 0)
            max_y = Min(max_y, road_edge_left_);

        if (Abs(track.yRel) > max_y)
            continue;

        if (speed_ms_ > 0.1F)
        {
            float abs_vel = Abs(track.vRel + speed_ms_) * 3.6F;
            if (abs_vel <= 5.0F)
            {
                continue;
            }
        }

        float dist = track.dRel + RADAR_TO_CAMERA + CAR_LENGTH;
        Vector3 pos(-track.yRel, 0, dist);
        live_tracks_.Push(pos);
        track_nodes_[index]->SetPosition(pos);

        if (track.vRel <= -speed_)
        {
            apply_rotation(track_nodes_[index], 180);
        }
        else
        {
            apply_rotation(track_nodes_[index], 0);
        }
        ++index;
    }
}

void Game::HandleOPModel(const cereal_Event& eventd)
{
    cereal_ModelDataV2 modelV2;
    cereal_read_ModelDataV2(&modelV2, eventd.modelV2);

    read_points(modelV2.position, path_vis_.points);

    model_.max_distance = Min(path_vis_.points.Back().z_ + CAR_LENGTH, MAX_DRAW_DISTANCE);

    // URHO3D_LOGINFOF("model_.max_distance=%f \n", model_.max_distance);

    for (int i = 0; i < 2; ++i)
        model_.edges[i].std = 1.0F;

    int len = capn_len(modelV2.roadEdges);
    capn_resolve(&modelV2.roadEdgeStds.p);
    capn_resolve(&modelV2.roadEdges.p);

    float road_edge_start[2] = {10.0, 10.0F};
    for (int i = 0; i < len; ++i)
    {
        model_.edges[i].std = capn_to_f32(capn_get32(modelV2.roadEdgeStds, i));
        cereal_ModelDataV2_XYZTData xyzt_data;
        cereal_get_ModelDataV2_XYZTData(&xyzt_data, modelV2.roadEdges, i);
        read_points(xyzt_data, road_edge_vis_[i].points);

        auto prob = 1.0 - model_.edges[i].std;
        if (!road_edge_vis_[i].points.Empty() && prob > 0.3)
            road_edge_start[i] = Abs(road_edge_vis_[i].points[0].x_);
        // printf("%d edge.points=%d std=%f \n", i, road_edge_vis_[i].points.Size(), model_.edges[i].std);
    }

    road_edge_left_ = road_edge_start[0];
    road_edge_right_ = road_edge_start[1];

    for (int i = 0; i < 4; ++i)
    {
        model_.lanes[i].std = 1.0F;
        model_.lanes[i].prob = 0.0F;
    }

    len = capn_len(modelV2.laneLineProbs);
    capn_resolve(&modelV2.laneLineProbs.p);
    capn_resolve(&modelV2.laneLineStds.p);
    capn_resolve(&modelV2.laneLines.p);

    for (int i = 0; i < len; ++i)
    {
        model_.lanes[i].prob = capn_to_f32(capn_get32(modelV2.laneLineProbs, i));
        model_.lanes[i].std = capn_to_f32(capn_get32(modelV2.laneLineStds, i));

        cereal_ModelDataV2_XYZTData xyzt_data;
        cereal_get_ModelDataV2_XYZTData(&xyzt_data, modelV2.laneLines, i);

        auto& points = lane_vis_[i].points;
        read_points(xyzt_data, points);

        // URHO3D_LOGINFOF("%d lanes.points=%d prob=%f ", i, points.Size(), model_.lanes[i].prob);
        // if (!points.Empty())
        // {
        //     auto pos = points[points.Size() / 2];
        //     URHO3D_LOGINFOF("%d pos=%s", i, pos.ToString().CString());
        // }
    }

    model_changed_ = true;
}

void Game::HandleOPControlState(const cereal_Event& eventd)
{
    struct cereal_ControlsState data;
    cereal_read_ControlsState(&data, eventd.controlsState);
    // URHO3D_LOGINFOF("data.vCruise=%f ", data.vCruise);
    set_speed_ = data.vCruise;

    static char buf[1024];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, data.alertText1.str, data.alertText1.len);
    String alertText1 = buf;

    memset(buf, 0, sizeof(buf));
    memcpy(buf, data.alertText2.str, data.alertText2.len);
    String alertText2 = buf;

    status_text_->SetText(alertText1 + "\n" + alertText2);

    lc_state_ = (alertText1.StartsWith("Changing Lane"));
    status_text_time_out_ = LC_ANIM_TIMER;
}

void Game::TriggerLaneChange(int lc_dir)
{
    lc_dir_ = lc_dir;
    lc_send_frames_ = 5;
}

void Game::UpdateNavigation(float dt)
{
    lc_navigation_ = 0;
    const auto& nav_data = nav_data_main_thread_;
    auto icon = nav_data.navi_icon;
    bool is_turn_right = (icon == NAVI_ICON_RIGHT) || (icon == NAVI_ICON_RIGHT_FRONT) || (icon == NAVI_ICON_RIGHT_BACK);
    bool has_exit = !nav_data.exit_name.Empty();
    auto dist = nav_data.dist_to_next_step;
    if (has_exit && dist > 0 && dist < 100 && is_turn_right)
    {
        // right lane line exist and clear
        if (model_.lanes[3].prob >= LANE_CHANGE_LINE_PROB && control_enabled_)
        {
            status_text_time_out_ = LC_ANIM_TIMER;
            status_text_->SetText(String("click right lane change button to off the ramp (distance:") + String(dist) +
                                  " m)");
            lc_navigation_ = -1;
        }
        else
        {
            status_text_time_out_ = LC_ANIM_TIMER;
            status_text_->SetText("dist to off the ramp: " + String(dist) + " m");
        }
    }
}
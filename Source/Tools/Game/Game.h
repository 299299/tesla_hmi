#pragma once

#include <vector>

#include <Urho3D/Urho3DAll.h>

#include "Sample.h"
#include "capn/log.capnp.h"
#include "messaging.hpp"
#include "modeldata.h"
#include "utils.hpp"

using namespace Urho3D;

namespace OP
{
class Context;
class Message;
class Poller;
class PubSocket;
class SubSocket;
};  // namespace OP

struct LineVisData
{
    PODVector< Vector3 > points;
    CustomGeometry* geometry;
    Node* node;
    SharedPtr< Material > material;
    float width;
    bool solid = true;
};

struct ConfigData
{
    bool hdr = false;
    bool debug = false;
    bool shadow = false;
    bool water_plane = false;

    // int day_color[3] = { 255, 255, 255 };
    // int night_color[3] = { 30, 30, 30 };
    int day_color[3] = {255, 255, 255};
    int night_color[3] = {60, 60, 60};

    float lane_color[3] = {140.0 / 255.0, 140.0 / 255.0, 140.0 / 255.0};
    float path_color[4] = {15.0 / 255.0, 188.0 / 255.0, 249.0 / 255.0, 0.9};
    float edge_color[4] = {255.0 / 255.0, 5.0 / 255.0, 5.0 / 255.0};

    float camera_min_dist = 4.0;
    float camera_max_dist = 50.0;

    float camera_init_dist = 20.0F;
    float camera_init_pitch = 16.0F;
    float camera_reset_time = 5.0F;

    bool thumbnail = false;
    bool is_night = false;

    int debug_text_size = 30;
    int speed_text_size = 120;
    int status_text_size = 25;
    int icon_size = 120;
    int indicator_size = 60;
    int speed_limit_size = 120;
    int lane_offset_size = 90;
    int debug_ui_size = 50;

    int left_gap = 20;
    int right_gap = 20;
    int top_gap = 20;

    int normal_gap = 20;

    float path_line_width = 0.25F;
    float lane_line_width = 0.065F;
    float road_edge_width = 0.05F;

    float min_lane_offset = -0.5F;
    float max_lane_offset = 0.5F;
    float gap_lane_offset = 0.1F;
};

struct NavData
{
    String exit_name;
    String cur_road_name;
    int speed_limit = 0;
    int navi_type = -1;
    int navi_icon = 0;
    int map_type = -1;
    int dist_to_next_step = -1;
    int remain_dist = -1;
    bool high_way = false;
};

#define MAX_LIVE_TRACKS 16

class Game : public Sample
{
    URHO3D_OBJECT(Game, Sample);

  public:
    Game(Context* context);
    ~Game();

    void Start() override;
    void Stop() override;
    void OnAndroidCallback(int msg, double data1, double data2, double data3, const char* data4);

  protected:
    void HandleSceneUpdate(StringHash eventType, VariantMap& eventData);
    void ReceiveDataFromOP();

    void SyncToOP();
    void InitOP();
    void SyncUI(float timeStep);
    void CreateScene();
    void Update(float timeStep);
    void UpdateInput(float timeStep);
    void UpdateDayLight();
    void CreateUI();
    void SetHDR(bool hdr);
    void UpdateLaneColor();
    void UpdateViewport();
    Node* CreateCarModel(const char* name, const char* mat_name);

    void HandleOPMessage(OP::Message* msg);
    void HandleCustomMessage(SharedPtr< JSONFile > json);
    void UpdateDebugTouch(float dt);

    void UpdateFPSCamera(float dt);
    void UpdateTPCamera(float dt);
    void UpdateFixedCamera(float dt);
    void UpdateKeyInput(float dt);
    void UpdateNavigation(float dt);

    void DrawDebug();
    void Draw3D(float dt);
    void Draw2D(float dt);

    void HandleOPModel(const cereal_Event& eventd);
    void HandleOPCarState(const cereal_Event& eventd);
    void HandleOPLiveTracks(cereal_Event& eventd);
    void HandleOPRadarState(const cereal_Event& eventd);
    void HandleOPControlState(const cereal_Event& eventd);

    void HandleTouchEnd(StringHash eventType, VariantMap& eventData);
    void HandleControlClicked(StringHash eventType, VariantMap& eventData);
    void HandleMouseButtonUp(StringHash eventType, VariantMap& eventData);

    void OnUIClicked(UIElement* e);
    void TriggerLaneChange(int lc_dir);

  private:
    String op_ip_address_;
    OP::Context* op_ctx_;
    OP::Poller* op_poller_;
    OP::PubSocket* sync_pub_;

    float last_sync_time_;
    ModelData model_;

    int op_status_;

    // Lane line visualization & points
    LineVisData lane_vis_[4];

    // path visualization & points
    LineVisData path_vis_;

    // road edge visualization & points
    LineVisData road_edge_vis_[2];

    Node* lead_car_;
    Node* lead_car2_;

    Light* front_light_;
    Light* tail_light_;
    Zone* zone_;

    SharedPtr< RenderPath > render_path_;

    Text* debug_text_;
    Text* status_text_;
    Text* speed_text_;
    Text* speed_hint_text_;
    Text* set_speed_text_;

    Text* gear_p_text_;
    Text* gear_r_text_;
    Text* gear_n_text_;
    Text* gear_d_text_;

    std::vector< OP::SubSocket* > socks_;
    bool model_changed_;

    float message_time_;

    ConfigData config_;

    float speed_;
    float speed_ms_;
    double longitude_;
    double latitude_;
    float touch_down_time_;
    int debug_touch_flag_;
    bool android_;
    unsigned num_cpu_cores_;
    bool brake_lights_;

    Sprite* left_turn_signal_sprite_;
    Sprite* right_turn_signal_sprite_;
    float turn_signal_time_;
    int turn_signal_;

    Mutex lock_;
    NavData nav_data_java_thread_;
    NavData nav_data_main_thread_;

    Sprite* speed_limit_sprite_;
    int last_speed_limit_;

    float camera_dist_;
    float target_pitch_;
    float target_dist_;
    float target_yaw_;
    float yaw_;
    float pitch_;
    float touch_up_time_;
    int camera_state_;
    float camera_blend_speed_;
    float camera_blend_acceleration_;

    Urho3D::String sync_str_;

    PODVector< char > json_buffer_;

    bool control_enabled_;

    Sprite* ad_on_sprite_;
    Sprite* ad_off_sprite_;

    PODVector< Vector3 > live_tracks_;
    Node* track_nodes_[MAX_LIVE_TRACKS];

    bool show_live_tracks_;

    Input* input_;
    Time* time_;
    Graphics* graphics_;
    UI* ui_;
    Renderer* render_;
    ResourceCache* cache_;

    int op_debug_mode_;
    float set_speed_;

    int model_frame_;
    float steering_wheel_;
    int car_gear_;

    PODVector< UIElement* > ui_elements_;

    Text* debug_setting_btn_;
    Text* debug_clean_data_btn_;
    Text* debug_no_log_btn_;
    Text* debug_log_btn_;
    PODVector< UIElement* > debug_ui_elements_;

    String op_cmd_line_;

    Sprite* left_lc_btn_;
    Sprite* right_lc_btn_;

    int lc_dir_;
    int lc_send_frames_;

    float road_edge_left_;
    float road_edge_right_;

    Node* ego_node_;
    SharedPtr< Material > ego_mat_;

    float status_text_time_out_;

    bool debug_test_;
    bool navigation_exit_;
};

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

struct Slot
{
    PODVector< Vector3 > points;
};

struct VehicleStatus
{
    bool ad_on = false;
    float pos_x = 0.0F;
    float pos_y = 0.0F;
    float yaw = 0.0F;
    float steering_wheel = 0.0F;
    float speed_kmh = 0.0F;
    int gear = 0;
    int turn_signal = 0;
    bool brake_lights = false;
    int parking_state = 0;

    Slot dest_slot;
    Slot trajectory;

    std::vector< Slot > slots;
};

struct ConfigData
{
    bool hdr = false;
    bool debug = false;
    bool shadow = false;
    bool water_plane = false;

    // int day_color[3] = { 255, 255, 255 };
    // int night_color[3] = { 30, 30, 30 };
    int day_color[3] = {185, 185, 185};
    int night_color[3] = {60, 60, 60};

    float camera_min_dist = 4.0;
    float camera_max_dist = 50.0;

    float camera_init_dist = 30.0F;
    float camera_init_pitch = 16.0F;
    float camera_reset_time = 5.0F;
    float camera_init_pitch_tp = 75.0F;

    bool thumbnail = false;
    bool is_night = false;

    int debug_text_size = 30;
    int speed_text_size = 300;
    int status_text_size = 30;
    int icon_size = 240;
    int button_width = 400;
    float button_r_w_h = 2.9F;
    int indicator_size = 240;
    int debug_ui_size = 50;
    int gear_ui_size = 100;

    int left_gap = 20;
    int right_gap = 20;
    int top_gap = 20;
    int normal_gap = 20;
};

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
    void UpdateViewport();
    Node* CreateCarModel(const char* name, const char* mat_name);

    void HandleCustomMessage(SharedPtr< JSONFile > json);
    void UpdateDebugTouch(float dt);

    void UpdateFPSCamera(float dt);
    void UpdateTPCamera(float dt);
    void UpdateFixedCamera(float dt);
    void UpdateKeyInput(float dt);

    void DrawDebug();
    void Draw3D(float dt);
    void Draw2D(float dt);
    void DrawSlots(float dt);
    void DrawMotionPlanning(float dt);

    void HandleTouchEnd(StringHash eventType, VariantMap& eventData);
    void HandleControlClicked(StringHash eventType, VariantMap& eventData);
    void HandleMouseButtonUp(StringHash eventType, VariantMap& eventData);

    void OnUIClicked(UIElement* e);

    bool Raycast(int x, int y, float maxDistance, Vector3& hitPos, Drawable*& hitDrawable);

    void PickSlot(int x, int y);

  private:
    String op_ip_address_;
    OP::Context* op_ctx_;
    OP::Poller* op_poller_;
    OP::PubSocket* sync_pub_;

    float last_sync_time_;

    int op_status_;

    Light* front_light_;
    Light* tail_light_;
    Zone* zone_;

    SharedPtr< RenderPath > render_path_;

    Text* debug_text_;
    Text* status_text_;
    Text* speed_text_;
    Text* speed_hint_text_;

    Text* gear_p_text_;
    Text* gear_r_text_;
    Text* gear_n_text_;
    Text* gear_d_text_;

    std::vector< OP::SubSocket* > socks_;

    float message_time_;

    ConfigData config_;

    float speed_;
    float speed_ms_;
    float touch_down_time_;
    int debug_touch_flag_;
    bool android_;
    unsigned num_cpu_cores_;

    Sprite* left_turn_signal_sprite_;
    Sprite* right_turn_signal_sprite_;
    float turn_signal_time_;

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

    Sprite* ad_on_sprite_;
    Sprite* ad_off_sprite_;

    Input* input_;
    Time* time_;
    Graphics* graphics_;
    UI* ui_;
    Renderer* render_;
    ResourceCache* cache_;

    int op_debug_mode_;

    VehicleStatus car_status_;

    PODVector< UIElement* > ui_elements_;

    Text* debug_setting_btn_;
    Text* debug_clean_data_btn_;
    Text* debug_no_log_btn_;
    Text* debug_log_btn_;
    PODVector< UIElement* > debug_ui_elements_;

    Node* ego_node_;
    SharedPtr< Material > ego_mat_;

    bool debug_test_;

    Node* parking_node_;
    std::vector< Node* > slot_nodes_;
    Node* dest_node_;
    Node* trajectory_node_;

    Material* parking_mat_;
    Material* parking_slot_mat_;

    Material* parking_sel_mat_;
    Material* parking_slot_sel_mat_;

    Vector3 last_pick_pos_ = Vector3(-999.0F, 0, -999.0F);
    Sprite* start_button_;
    int parking_button_clicked_ = 0;
};

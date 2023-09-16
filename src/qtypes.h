#ifndef QTYPES_H
#define QTYPES_H
#include <stdbool.h>
#include <stdint.h>
typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

typedef unsigned char byte_vec4_t[4];
// cplane_t structure
typedef struct cplane_s {
  vec3_t normal;
  float dist;
  short type;     // for fast side tests
  short signbits; // signx + (signy<<1) + (signz<<1)
} cplane_t;
// a trace is returned when a box is swept through the world
typedef struct {
  int allsolid;   // if true, plane is not valid
  int startsolid; // if true, the initial point was in a solid area
  float fraction; // time completed, 1.0 = didn't hit anything
  vec3_t endpos;  // final position
  cplane_t plane; // surface normal at impact
  int surfFlags;  // surface hit
  int contents;   // contents on other side of surface hit
  int ent;        // not set by CM_*() functions
} trace_t;
typedef struct {
  float dualquat[8];
} bonepose_t;

typedef enum {
  RT_MODEL,
  RT_SPRITE,
  RT_PORTALSURFACE,
  NUM_RTYPES
} refEntityType_t;

typedef struct {
  refEntityType_t rtype;
  union {
    int flags;
    int renderfx;
  };
  struct model_s *model;
  float axis[9];
  vec3_t origin;
  vec3_t origin2;
  vec3_t lightingOrigin;
  int frame;
  bonepose_t *boneposes;
  int oldframe;
  bonepose_t *oldboneposes;
  float backlerp;
  struct skinfile_s *customSkin;
  struct shader_s *customShader;
  unsigned int shaderTime;
  union {
    byte_vec4_t color;
    uint8_t shaderRGBA[4];
  };
  float scale;
  float radius;
  float rotation;
  float outlineHeight;
  union {
    byte_vec4_t outlineColor;
    uint8_t outlineRGBA[4];
  };
} entity_t;

typedef struct {
  int number;
  unsigned int svflags;
  int type;
  int linearMovement;
  union {
    vec3_t linearMovementVelocity;
    vec3_t linearMovementEnd;
  };
  vec3_t origin;
  vec3_t angles;
  union {
    vec3_t old_origin;
    vec3_t origin2;
    vec3_t linearMovementBegin;
  };
  unsigned int modelindex;
  union {
    unsigned int modelindex2;
    int bodyOwner;
    int channel;
  };
  union {
    int frame;
    int ownerNum;
  };
  union {
    int counterNum;
    int skinnum;
    int itemNum;
    int firemode;
    int damage;
    int targetNum;
    int colorRGBA;
    int range;
    unsigned int linearMovementDuration;
  };
  float attenuation;
  int weapon;
  int teleported;
  unsigned int effects;
  union {
    int solid;
    int eventCount;
  };
  int sound;
  int events[2];
  int eventParms[2];
  union {
    unsigned int linearMovementTimeStamp;
    int light;
  };
  int team;
} entity_state_t;

typedef enum {
  IT_WEAPON = 1,
  IT_AMMO,
  IT_ARMOR = 4,
  IT_POWERUP = 8,
  IT_HEALTH = 64
} itemtype_t;
typedef struct {
  char *classname;
  int tag;
  itemtype_t type;
  int flags;
  char *world_model[2];
  char *icon;
  char *simpleitem;
  char *pickup_sound;
  int effects;
  char *name;
  char *shortname;
  char *color;
  int quantity;
  int inventory_max;
  int ammo_tag;
  int weakammo_tag;
  void *info;
  char *precache_models;
  char *precache_sounds;
  char *precache_images;
} gsitem_t;

typedef struct {
  entity_state_t current;
  entity_state_t prev;
  int serverFrame;
  unsigned int fly_stoptime;
  int respawnTime;
  entity_t ent;
  unsigned int type;
  unsigned int renderfx;
  unsigned int effects;
  struct cgs_skeleton_s *skel;
  vec3_t velocity;
  bool canExtrapolate;
  bool canExtrapolatePrev;
  vec3_t prevVelocity;
  int microSmooth;
  vec3_t microSmoothOrigin;
  vec3_t microSmoothOrigin2;
  gsitem_t *item;
  vec3_t trailOrigin;
  unsigned int localEffects[64];
  vec3_t laserOrigin;
  vec3_t laserPoint;
  vec3_t laserOriginOld;
  vec3_t laserPointOld;
  bool laserCurved;
  bool linearProjectileCanDraw;
  vec3_t linearProjectileViewerSource;
  vec3_t linearProjectileViewerVelocity;
  vec3_t teleportedTo;
  vec3_t teleportedFrom;
  byte_vec4_t outlineColor;
  bool pendingAnimationsUpdate;
  int lastAnims;
  int lastVelocitiesFrames[4];
  float lastVelocities[4][4];
  bool jumpedLeft;
  vec3_t animVelocity;
  float yawVelocity;
  struct cinematics_s *cin;
} centity_t;
typedef struct {
  int pm_type;
  float origin[3];
  float velocity[3];
  int pm_flags;
  int pm_time;
  short stats[16];
  int gravity;
  short delta_angles[3];
} pmove_state_t;
typedef struct {
  pmove_state_t pmove;
  vec3_t viewangles;
  int event[2];
  int eventParm[2];
  unsigned int POVnum;
  unsigned int playerNum;
  float viewheight;
  float fov;
  uint8_t weaponState;
  int inventory[64];
  short stats[64];
  uint8_t plrkeys;
} player_state_t;
typedef struct {
  char name[64];
  char cleanname[64];
  int hand;
  byte_vec4_t color;
  struct shader_s *icon;
  int modelindex;
} cg_clientInfo_t;
typedef struct {
  int x;
  int y;
  int width;
  int height;
  int scissor_x;
  int scissor_y;
  int scissor_width;
  int scissor_height;
  int ortho_x;
  int ortho_y;
  float fov_x;
  float fov_y;
  vec3_t vieworg;
  float viewaxis[9];
  float blend[4];
  unsigned int time;
  int rdflags;
  // skyportal_t
  char skyportal[36];
  uint8_t *areabits;
  float weaponAlpha;
  float minLight;
  struct shader_s *colorCorrection;
} refdef_t;
typedef struct {
  int type;
  int POVent;
  bool thirdperson;
  bool playerPrediction;
  bool drawWeapon;
  bool draw2D;
  refdef_t refdef;
  float fracDistFOV;
  vec3_t origin;
  vec3_t angles;
  float axis[9];
  vec3_t velocity;
  bool flipped;
} cg_viewdef_t;
typedef struct {
  unsigned int time;
  float delay;
  unsigned int realTime;
  float frameTime;
  float realFrameTime;
  int frameCount;
  unsigned int firstViewRealTime;
  int viewFrameCount;
  bool startedMusic;
  char wtf[7];        /// ???????? this type shouldn't exist
  char frame[292480]; // these aren't actually arrays, just giant typedefs.
                      // skipping
  char oldFrame[292480];
  // snapshot_t frame;
  // snapshot_t oldFrame;
  bool frameSequenceRunning;
  bool oldAreabits;
  bool portalInView;
  bool fireEvents;
  bool firstFrame;
  float predictedOrigins[64][3];
  float predictedStep;
  unsigned int predictedStepTime;
  unsigned int predictingTimeStamp;
  unsigned int predictedEventTimes[32];
  vec3_t predictionError;
  player_state_t predictedPlayerState;
  // int predictedWeaponSwitch;
  // int predictedGroundEntity;
  // gs_laserbeamtrail_t weaklaserTrail;
  // int predictFrom;
  // entity_state_t predictFromEntityState;
  // player_state_t predictFromPlayerState;
  // int lastWeapon;
  // unsigned int lastCrossWeapons;
  // mat3_t autorotateAxis;
  // float lerpfrac;
  // float xerpTime;
  // float oldXerpTime;
  // float xerpSmoothFrac;
  // int effects;
  // vec3_t lightingOrigin;
  // bool showScoreboard;
  // bool specStateChanged;
  // unsigned int multiviewPlayerNum;
  // int pointedNum;
  // float xyspeed;
  // float oldBobTime;
  // int bobCycle;
  // float bobFracSin;
  // cg_kickangles_t kickangles[3];
  // cg_viewblend_t colorblends[3];
  // unsigned int damageBlends[4];
  // unsigned int fallEffectTime;
  // unsigned int fallEffectRebounceTime;
  // const char *matchmessage;
  // char helpmessage[4096];
  // unsigned int helpmessage_time;
  // char *teaminfo;
  // size_t teaminfo_size;
  // char *motd;
  // unsigned int motd_time;
  // char quickmenu[1024];
  // bool quickmenu_left;
  // char award_lines[3][64];
  // unsigned int award_times[3];
  // int award_head;
  // struct cg_layoutnode_s *statusBar;
  // cg_viewweapon_t weapon;
  // cg_viewdef_t view;
  // cg_gamechat_t chat;
} cg_state_t;
typedef struct cvar_s {
  char *name;
  char *string;
  char *dvalue;
  char *latched_string; // for CVAR_LATCH vars
  int flags;
  int modified; // set each time the cvar is changed
  float value;
  int integer;
} cvar_t;
#endif

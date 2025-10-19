#include <stdint.h>

//Offset for Volume Identifier
#define offset_v_entry_type 0X00
#define offset_v_unused 0X01
#define offset_v_time_stamp 0X04
#define offset_v_name 0X0C

//Offset for Starting Marker Entry
#define offset_sm_type 0X00
#define offset_sm_unused 0X01

//Offset for Unused Entry
#define offset_u_type 0X00
#define offset_u_unused 0X01

//Offset for Directory Entry
#define offset_d_type 0X00
#define offset_d_cont 0X01
#define offset_d_time_stamp 0X02
#define offset_d_name 0X0A

//Offset for File Entry
#define offset_f_type 0X00
#define offset_f_cont 0X01
#define offset_f_time_stamp 0X02
#define offset_f_start 0X0A
#define offset_f_end 0X12
#define offset_f_length 0X1A
#define offset_f_name 0X22

//Offset for Unusable Entry
#define offset_uu_type 0x00
#define offset_uu_unused 0x01
#define offset_uu_start 0x0A
#define offset_uu_end 0X12
#define offset_uu_unused2 0X1A

//Offset for Deleted Directory Entry
#define offset_dd_entry 0X00    
#define offset_dd_cont 0X01
#define offset_dd_time_stamp 0X02
#define offset_dd_name 0x0A

//Offset for Deleted File Entry
#define offset_df_type 0x00
#define offset_df_cont 0x01
#define offset_df_time_stamp 0x02
#define offset_df_start 0x0A
#define offset_df_end 0x12
#define offset_df_length 0x1A
#define offset_df_name 0x22

//Offset for Continuation Entry
#define offset_c_entry 0x00


typedef struct volume_identifier
{
    uint8_t type;
    uint8_t unused[3];
    uint64_t time_stamp;
    uint8_t name[52];
};

typedef struct starting_marker
{
    uint8_t type;
    uint8_t unused[63];
};

typedef struct unused_entry
{
    uint8_t type;
    uint8_t unused[63];
};

typedef struct directory_entry
{
   uint8_t type;
   uint8_t number_entries;
   uint64_t time_stamp;
   uint8_t name[54];
};

typedef struct file_entry
{
    uint8_t type;
    uint8_t number_entries;
    uint64_t time_stamp;
    uint64_t start;
    uint64_t end;
    uint64_t length;
    uint8_t name[30];
};

typedef struct unusable_entry
{
    uint8_t type;
    uint8_t unused[9];
    uint64_t start;
    uint64_t end;
    uint8_t unused_1[38];

};

typedef struct deleted_directory
{
    uint8_t type;
    uint8_t number_entries;
    uint64_t time_stamp;
    uint8_t name[54];
};

typedef struct deleted_file
{
    uint8_t type;
    uint8_t number_entries;
    uint64_t time_stamp;
    uint64_t start;
    uint64_t end;
    uint64_t length;
    uint8_t name[30];
};

typedef struct contiunation
{
    uint8_t name[64];
};




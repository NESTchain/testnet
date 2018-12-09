#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @defgroup types Builtin Types
 *  @ingroup contractdev
 *  @brief Specifies typedefs and aliases
 *
 *  @{
 */

#define GRAPHENE_DB_MAX_INSTANCE_ID  (uint64_t(-1)>>16)

/**
 * @brief Name of an account
 * @details Name of an account
 */
typedef uint64_t account_name;

/**
 * @brief Name of a permission
 * @details Name of a permission
 */
typedef uint64_t permission_name;

/**
 * @brief Name of a table
 * @details Name of a table
 */
typedef uint64_t table_name;

/**
 * @brief Time
 * @details Time
 */
typedef uint32_t time;

/**
 * @brief Name of a scope
 * @details Name of a scope
 */
typedef uint64_t scope_name;

/**
 * @brief Name of an action
 * @details Name of an action
 */
typedef uint64_t action_name;

/**
 * @brief Macro to align/overalign a type to ensure calls to intrinsics with pointers/references are properly aligned
 * @details Macro to align/overalign a type to ensure calls to intrinsics with pointers/references are properly aligned
 */

typedef uint16_t weight_type;

/* macro to align/overalign a type to ensure calls to intrinsics with pointers/references are properly aligned */
#define ALIGNED(X) __attribute__ ((aligned (16))) X

struct public_key {
   char data[33];
};

struct signature {
   uint8_t data[65];
};

struct ALIGNED(checksum256) {
   uint8_t hash[32];
};

struct ALIGNED(checksum160) {
   uint8_t hash[20];
};

struct ALIGNED(checksum512) {
   uint8_t hash[64];
};

typedef struct checksum160      block_id_type;

#ifdef __cplusplus
} /// extern "C"
#endif
/// @}

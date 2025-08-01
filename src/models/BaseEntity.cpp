#include "../../include/models/BaseEntity.h"
#include <uuid/uuid.h>
#include <sstream>
#include <iomanip>

namespace healthcare::models {

BaseEntity::BaseEntity() 
    : id_(generateUUID()),
      created_at_(std::chrono::system_clock::now()),
      updated_at_(std::chrono::system_clock::now()),
      is_deleted_(false) {
}

std::string BaseEntity::generateUUID() {
    uuid_t uuid;
    uuid_generate_random(uuid);
    
    char uuid_str[37];
    uuid_unparse_lower(uuid, uuid_str);
    
    return std::string(uuid_str);
}

} // namespace healthcare::models
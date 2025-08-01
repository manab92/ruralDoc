#pragma once

#include "BaseRepository.h"
#include "../models/Doctor.h"

namespace healthcare::database {

class DoctorRepository : public BaseRepository<models::Doctor> {
public:
    DoctorRepository() : BaseRepository("doctors") {}
    ~DoctorRepository() = default;
    
    // Custom queries
    QueryResult<models::Doctor> findByUserId(const std::string& user_id);
    QueryResult<models::Doctor> findBySpecialization(const std::string& specialization);
    QueryResult<models::Doctor> findByClinic(const std::string& clinic_id);
    QueryResult<models::Doctor> findByCity(const std::string& city);
    QueryResult<models::Doctor> findAvailableDoctors(const std::chrono::system_clock::time_point& date_time);
    QueryResult<models::Doctor> findByConsultationType(models::ConsultationType type);
    QueryResult<models::Doctor> findVerifiedDoctors();
    
    // Search
    QueryResult<models::Doctor> searchDoctors(const std::string& query);
    QueryResult<models::Doctor> findNearby(double latitude, double longitude, double radius_km);
    
    // Validation
    bool licenseNumberExists(const std::string& license_number);
    
    // Statistics
    int countBySpecialization(const std::string& specialization);
    int countVerifiedDoctors();
    std::map<std::string, int> getDoctorStatsByCity();
    
protected:
    // Override BaseRepository methods
    models::Doctor mapRowToEntity(const pqxx::row& row) const override;
    std::vector<std::string> getInsertValues(const models::Doctor& entity) const override;
    std::vector<std::string> getUpdateValues(const models::Doctor& entity) const override;
    std::vector<std::string> getColumnNames() const override;
    std::vector<std::string> getSearchableColumns() const override;
};

} // namespace healthcare::database
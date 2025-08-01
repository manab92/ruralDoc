#pragma once

#include "BaseRepository.h"
#include "../models/Appointment.h"

namespace healthcare::database {

class AppointmentRepository : public BaseRepository<models::Appointment> {
public:
    AppointmentRepository() : BaseRepository("appointments") {}
    ~AppointmentRepository() = default;
    
    // Custom queries
    QueryResult<models::Appointment> findByUserId(const std::string& user_id);
    QueryResult<models::Appointment> findByDoctorId(const std::string& doctor_id);
    QueryResult<models::Appointment> findByClinicId(const std::string& clinic_id);
    QueryResult<models::Appointment> findByDateRange(const std::chrono::system_clock::time_point& start,
                                                     const std::chrono::system_clock::time_point& end);
    QueryResult<models::Appointment> findUpcomingAppointments(const std::string& user_id);
    QueryResult<models::Appointment> findByStatus(models::AppointmentStatus status);
    
    // Check availability
    bool isTimeSlotAvailable(const std::string& doctor_id,
                           const std::chrono::system_clock::time_point& start_time,
                           const std::chrono::system_clock::time_point& end_time);
    
    // Statistics
    int countByDoctor(const std::string& doctor_id);
    int countByClinic(const std::string& clinic_id);
    std::map<std::string, int> getAppointmentStatsByStatus();
    
protected:
    // Override BaseRepository methods
    models::Appointment mapRowToEntity(const pqxx::row& row) const override;
    std::vector<std::string> getInsertValues(const models::Appointment& entity) const override;
    std::vector<std::string> getUpdateValues(const models::Appointment& entity) const override;
    std::vector<std::string> getColumnNames() const override;
    std::vector<std::string> getSearchableColumns() const override;
};

} // namespace healthcare::database
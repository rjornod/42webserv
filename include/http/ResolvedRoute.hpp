#pragma once
#include "LocationConfig.hpp"

class ResolvedRoute {

  private:
    LocationConfig m_locationConfig;

  public:
    ResolvedRoute() {};
    
    LocationConfig getLocationConfig() const {return m_locationConfig;}
    void           setLocationConfig(const LocationConfig& locationConfig) {m_locationConfig = locationConfig;}
};
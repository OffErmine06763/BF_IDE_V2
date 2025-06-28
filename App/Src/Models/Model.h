#pragma once
#include "Utility.h"

class Model {
public:
	std::unordered_map<i32, std::vector<callable>> m_Listeners;
};
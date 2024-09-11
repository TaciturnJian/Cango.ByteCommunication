project(Cango.ByteCommunication.BoostImplementations)

AddCXXModule(
	NAME "BoostImplementations"
	NAMESPACE "Cango::ByteCommunication"
	LINKS
		"fmt::fmt"
		"spdlog::spdlog"
		"Boost::system"
		"Cango::ByteCommunication::Core"
)

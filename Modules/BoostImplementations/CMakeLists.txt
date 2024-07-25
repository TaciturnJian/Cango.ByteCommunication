project(Cango.ByteCommunication.BoostImplementations)

AddCXXModule(
	NAME "BoostImplementations"
	NAMESPACE "Cango_ByteCommunication"
	LINKS
		"fmt::fmt"
		"spdlog::spdlog"
		"Boost::system"
		"Cango_ByteCommunication_Core"
)

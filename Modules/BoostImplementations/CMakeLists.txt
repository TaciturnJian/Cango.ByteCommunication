project(Cango.ByteCommunication.BoostImplementations)

AddCXXModule(
	NAME "BoostImplementations"
	NAMESPACE "Cango_ByteCommunication"
	LINKS
		"Boost::system"
		"Cango_ByteCommunication_Core"
		"spdlog::spdlog"
	
)

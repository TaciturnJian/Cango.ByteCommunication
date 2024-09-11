cmake_minimum_required(VERSION 3.28)

include("Cango")

project("Cango.ByteCommunication")

set(RequiredLibraries
	"Cango::TaskDesign"
)
RequireLibraries(RequiredLibraries)

AddModulesInDirectory(
	DIRECTORY "Modules"
	NAMES
		"Core"
		"BoostImplementations"
)

AddCXXModule(
	NAME "ByteCommunication"
	NAMESPACE "Cango"
	LINKS
		"Cango::ByteCommunication::Core"
		"Cango::ByteCommunication::BoostImplementations"
)

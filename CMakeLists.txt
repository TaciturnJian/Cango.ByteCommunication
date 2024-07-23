cmake_minimum_required(VERSION 3.28)

include(Cango)

project("Cango.ByteCommunication")

RequireLibraries(
	"Cango_TaskDesign"
)

AddModulesInDirectory(
	DIRECTORY "Modules"
	NAMES
		"Core"
		"BoostImplementations"
)

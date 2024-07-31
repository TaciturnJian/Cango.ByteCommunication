project(Cango.ByteCommunication.Core)

AddCXXModule(
	NAME "Core"
	NAMESPACE "Cango_ByteCommunication"
	LINKS
		"Cango::TaskDesign"
		"Cango_CommonUtils::AsyncItemPool"
)

project(Cango.ByteCommunication.Core)

AddCXXModule(
	NAME "Core"
	NAMESPACE "Cango::ByteCommunication"
	LINKS
		"Cango::TaskDesign"
		"Cango::CommonUtils::AsyncItemPool"
)

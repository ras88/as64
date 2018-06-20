add_executable(as64
	types.cpp
	str.cpp
	path.cpp
	enum.cpp
	error.cpp
	message.cpp
	source.cpp
	instruction.cpp
	buffer.cpp
	ast.cpp
	parser.cpp
	symbol.cpp
	define.cpp
	emit.cpp
	lister.cpp
	cmdline.cpp
	main.cpp
)

install (TARGETS as64 DESTINATION bin)

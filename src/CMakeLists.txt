add_executable(cassm
	types.cpp
	str.cpp
	enum.cpp
	error.cpp
	message.cpp
	source.cpp
	instruction.cpp
	buffer.cpp
	ast.cpp
	parser.cpp
	define.cpp
	main.cpp
)

install (TARGETS cassm DESTINATION bin)

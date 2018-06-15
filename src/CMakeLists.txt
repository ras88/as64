add_executable(cassm
	str.cpp
	error.cpp
	message.cpp
	source.cpp
	instruction.cpp
	buffer.cpp
	ast.cpp
	context.cpp
	parser.cpp
	main.cpp
)

install (TARGETS cassm DESTINATION bin)

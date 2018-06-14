add_executable(cassm
	str.cpp
	error.cpp
	source.cpp
	instruction.cpp
	buffer.cpp
	expr.cpp
	assembler.cpp
	main.cpp
)

install (TARGETS cassm DESTINATION bin)

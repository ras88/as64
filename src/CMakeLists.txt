add_executable(cassm
	str.cpp
	source.cpp
	instruction.cpp
	buffer.cpp
	main.cpp
)

install (TARGETS cassm DESTINATION bin)

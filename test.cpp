#include <iostream>
#include <string>

int main() {
	std::cout << "Enter Name: ";
	std::string name;
	std::getline(std::cin, name);
	std::cout << "Hello " << name << std::endl;
}
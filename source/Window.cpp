#include"Window.hpp"
#include<iostream>

Window::Window(const unsigned int& _width, const unsigned int& _height, std::string const& _title)
:	width(_width),
	height(_height),
	title(_title)
{
	window.create(sf::VideoMode(width, height), title);
	window.clear(sf::Color::Black);

	window.setActive(false);

	std::thread	thread(&Window::render_thread, this);

	sf::Event event;

	while (isWindowOpen() && !close_window.load()) {
		if (window.waitEvent(event)) {
			event_queue.push(event);
		}
	}
	if (thread.joinable())
		thread.join();
	window.setActive(true);
	window.close();
}

bool Window::isWindowOpen()
{
	std::lock_guard<std::mutex> fullscreen_lock(window_mutex);
	return window.isOpen();
}

void Window::render_thread() {
	sf::Texture test;
	if (!test.loadFromFile("preview.png")) {
		std::cout << "error loading image\n\r";
	}

	window.setActive();

	while (!close_window) {
		window.clear();

		while (auto event = event_queue.pop_if_not_empty())
		{
			switch (event->type) {
			case sf::Event::Closed:
				close_window.store(true);
				break;
			case sf::Event::KeyPressed: {
			}
			break;
			case sf::Event::Resized:
				break;
			}
		}
		window.display();
	}
}
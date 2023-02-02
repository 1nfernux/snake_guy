//!ƒЋя «јѕ”— ј “–≈Ѕ”ё“—я –≈—”–—џ (“≈ —“”–џ, —ЁћѕЋџ, Ў–»‘“, Ѕ»ЅЋ»ќ“≈ ј SFML, VISUAL STUDIO 17 √ќƒј (Ќј 19 Ќ≈“  ќћѕќЌ≈Ќ“ќ¬ ƒЋя SFML))!
#include <SFML/Graphics.hpp> //подключение библиотек sfml, cstdlib и vector
#include <SFML/Audio.hpp>
#include <cstdlib>
#include <vector>

#define FIELD_CELL_TYPE_NONE 0 //объ€вление глобальных переменных (тип клетки 0, €блоко, стена)
#define FIELD_CELL_TYPE_APPLE -1
#define FIELD_CELL_TYPE_WALL -2
#define SNAKE_DIRECTION_UP 0 //объ€вление глобальных переменных (направлений змейки)
#define SNAKE_DIRECTION_RIGHT 1
#define SNAKE_DIRECTION_DOWN 2
#define SNAKE_DIRECTION_LEFT 3
#define MENU_ITEM_START "Start new game" //объ€вление глобальных переменных (главное меню и настройки)
#define MENU_ITEM_RESUME "Resume game"
#define MENU_ITEM_SETTINGS "Settings"
#define MENU_ITEM_QUIT "Quit"
#define MENU_ITEM_BACK "Back to main menu"
#define MENU_ITEM_VOLUME "Volume"
#define MENU_MAIN 0
#define MENU_SETTINGS 1
#define LIVES 3 //объ€вление глобальных переменных (жизни)

const int field_size_x = 35; //задаем размеры пол€, клетки, размер счетчкика очков и окна
const int field_size_y = 25;
const int cell_size = 32;
const int score_bar_height = 60;
const int window_width = field_size_x * cell_size;
const int window_height = field_size_y * cell_size + score_bar_height;

struct GameState { //создаем структуру с основопологающими переменными такими как положение змейки, еЄ длина. создаем матрицу (поле)
    int field[field_size_y][field_size_x];
    int snake_position_x;
    int snake_position_y;
    int snake_length = 0;
    int snake_direction = SNAKE_DIRECTION_RIGHT;
    int score = 0;
};
GameState game_state;
std::vector<GameState> game_last_states;
bool rollback = false; //перемотка при потере жизни
int lives = 0;
bool game_started = false;
bool game_over = false;
int game_over_timeout = 0;
bool game_paused = true; //пауза
int current_menu = MENU_MAIN; //главное меню
int current_main_menu_item_index = 0;
int current_settings_menu_item_index = 0;
int settings_volume = 50; //звук по умолчанию

sf::Texture snake_texture; //объ€вл€ем главные текстуры, спрайты, сэмплы и шрифты
sf::Sprite snake;

sf::Image image;

sf::Texture head_texture;
sf::Sprite head;

sf::Texture none_texture;
sf::Sprite none;

sf::Texture apple_texture;
sf::Sprite apple;

sf::Texture wall_texture;
sf::Sprite wall;

sf::Texture life_texture;
sf::Sprite life;

sf::SoundBuffer sb_menu_navigate;
sf::Sound sound_menu_navigate;

sf::SoundBuffer sb_game_start;
sf::Sound sound_game_start;

sf::SoundBuffer sb_ate_apple;
sf::Sound sound_ate_apple;

sf::SoundBuffer sb_died_against_the_wall;
sf::Sound sound_died_against_the_wall;

sf::SoundBuffer sb_ate_himself;
sf::Sound sound_ate_himself;

sf::Font font_score;
sf::Text text_score;

sf::Font font_title;
sf::Text text_title;

sf::Font font_game_over;
sf::Text text_game_over;

sf::Font font_menu;
std::vector<sf::Text> text_main_menu_items;
std::vector<std::string> main_menu_items = { MENU_ITEM_START, MENU_ITEM_SETTINGS, MENU_ITEM_QUIT };
std::vector<sf::Text> text_settings_menu_items;
std::vector<std::string> settings_menu_items = { MENU_ITEM_BACK, MENU_ITEM_VOLUME };

int get_random_empty_cell() //заполнение пол€ пустыми клетками (там же считаем их количество)
{
    int empty_cell_count = 0;
    for (int j = 0; j < field_size_y; j++) {
        for (int i = 0; i < field_size_x; i++) {
            if (game_state.field[j][i] == FIELD_CELL_TYPE_NONE) {
                empty_cell_count++;
            }
        }
    }
    int target_empty_cell_index = std::rand() % empty_cell_count;
    int empty_cell_index = 0;
    for (int j = 0; j < field_size_y; j++) {
        for (int i = 0; i < field_size_x; i++) {
            if (game_state.field[j][i] == FIELD_CELL_TYPE_NONE) {
                if (empty_cell_index == target_empty_cell_index) {
                    return j * field_size_x + i;
                }
                empty_cell_index++;
            }
        }
    }
    return -1;
}

void add_apple() //создаем €блоко (если €блоко находитс€ в пустой клетке и если она находитс€ в поле)
{
    int apple_pos = get_random_empty_cell();
    if (apple_pos != -1) {
        game_state.field[apple_pos / field_size_x][apple_pos % field_size_x] = FIELD_CELL_TYPE_APPLE;
    }
}

void clear_field()//очищаем поле дл€ старта, заполн€€ его пустыми клетками (так же создаем стенки)
{
    for (int j = 0; j < field_size_y; j++) {
        for (int i = 0; i < field_size_x; i++) {
            game_state.field[j][i] = FIELD_CELL_TYPE_NONE;
        }
    }
    for (int i = 0; i < game_state.snake_length; i++) {
        game_state.field[game_state.snake_position_y][game_state.snake_position_x - i] = game_state.snake_length - i;
    }
    for (int i = 0; i < field_size_x; i++) {
        if (i < 5 || field_size_x - i - 1 < 5) {
            game_state.field[0][i] = FIELD_CELL_TYPE_WALL;
            game_state.field[field_size_y - 1][i] = FIELD_CELL_TYPE_WALL;
        }
    }
    for (int i = 1; i < field_size_y - 1; i++) {
        if (i < 5 || field_size_y - i - 1 < 5) {
            game_state.field[i][0] = FIELD_CELL_TYPE_WALL;
            game_state.field[i][field_size_x - 1] = FIELD_CELL_TYPE_WALL;
        }
    }
    add_apple();
}

void set_volume() //создаем настройку звука
{
    sound_menu_navigate.setVolume(settings_volume);
    sound_game_start.setVolume(settings_volume);
    sound_ate_apple.setVolume(settings_volume);
    sound_died_against_the_wall.setVolume(settings_volume);
    sound_ate_himself.setVolume(settings_volume);
}

void init_game() //указываем текстурам спрайтам сэмплам и шрифту местонахождение ресурсов
{
    std::srand(time(NULL));
    clear_field();

    snake_texture.loadFromFile("images/beard.png");
    snake.setTexture(snake_texture);

    image.loadFromFile("images/life.png");

    head_texture.loadFromFile("images/head.png");
    head.setTexture(head_texture);

    none_texture.loadFromFile("images/none.png");
    none.setTexture(none_texture);

    apple_texture.loadFromFile("images/apple.png");
    apple.setTexture(apple_texture);

    wall_texture.loadFromFile("images/wall.png");
    wall.setTexture(wall_texture);

    life_texture.loadFromFile("images/life.png");
    life.setTexture(life_texture);

    sb_menu_navigate.loadFromFile("audio/select.wav");
    sound_menu_navigate.setBuffer(sb_menu_navigate);

    sb_game_start.loadFromFile("audio/start.wav");
    sound_game_start.setBuffer(sb_game_start);

    sb_ate_apple.loadFromFile("audio/nyam.wav");
    sound_ate_apple.setBuffer(sb_ate_apple);

    sb_died_against_the_wall.loadFromFile("audio/death.wav");
    sound_died_against_the_wall.setBuffer(sb_died_against_the_wall);

    sb_ate_himself.loadFromFile("audio/death.wav");
    sound_ate_himself.setBuffer(sb_ate_himself);

    set_volume();

    font_score.loadFromFile("fonts/font.ttf");
    text_score.setFont(font_score);
    text_score.setCharacterSize(36);
    text_score.setFillColor(sf::Color::Black);

    font_title.loadFromFile("fonts/font.ttf");
    text_title.setFont(font_title);
    text_title.setString("Scientist");
    text_title.setCharacterSize(40);
    text_title.setFillColor(sf::Color::Black);
    text_title.setPosition(20, 0);

    font_game_over.loadFromFile("fonts/font.ttf");
    text_game_over.setFont(font_game_over);
    text_game_over.setString("GAME OVER");
    text_game_over.setCharacterSize(120);
    text_game_over.setFillColor(sf::Color::Red);
    text_game_over.setPosition((window_width - text_game_over.getLocalBounds().width) / 2, (window_height - text_game_over.getLocalBounds().height + score_bar_height) / 2);

    font_menu.loadFromFile("fonts/font.ttf");
    for (int i = 0; i < main_menu_items.size(); i++) {
        text_main_menu_items.emplace_back(sf::Text());
        text_main_menu_items.back().setString(main_menu_items.at(i));
        text_main_menu_items.back().setFont(font_menu);
        text_main_menu_items.back().setCharacterSize(40);
    }
    for (int i = 0; i < settings_menu_items.size(); i++) {
        text_settings_menu_items.emplace_back(sf::Text());
        text_settings_menu_items.back().setString(settings_menu_items.at(i));
        text_settings_menu_items.back().setFont(font_menu);
        text_settings_menu_items.back().setCharacterSize(40);
    }
}

void start_game() //задаем начальное положение змейке, еЄ направление, провер€ем всЄ дл€ начала игры
{
    game_state.snake_position_x = field_size_x / 2;
    game_state.snake_position_y = field_size_y / 2;
    game_state.snake_length = 4;
    game_state.snake_direction = SNAKE_DIRECTION_RIGHT;
    game_state.score = 0;
    game_started = true;
    game_over = false;
    game_paused = false;
    lives = LIVES;
    clear_field();
    sound_game_start.play();
}

void finish_game() //объ€вл€ем геймовер, конец игры
{
    game_over = true;
    game_paused = true;
    game_over_timeout = 20;
    current_main_menu_item_index = 0;
}

void snake_died() //откат змейки при потере жизни, если жизней нет, то конец игры
{
    lives--;
    if (lives > 0) {
        rollback = true;
    }
    else {
        finish_game();
    }
}

void draw_field(sf::RenderWindow& window) //рисуем поле, задаем типам клетки значени€, там же настраиваем управление змейкой
{
    for (int j = 0; j < field_size_y; j++) {
        for (int i = 0; i < field_size_x; i++) {
            switch (game_state.field[j][i]) {
            case FIELD_CELL_TYPE_NONE:
                none.setPosition(float(i * cell_size), float(j * cell_size + score_bar_height));
                window.draw(none);
                break;
            case FIELD_CELL_TYPE_APPLE:
                apple.setPosition(float(i * cell_size), float(j * cell_size + score_bar_height));
                window.draw(apple);
                break;
            case FIELD_CELL_TYPE_WALL:
                wall.setPosition(float(i * cell_size), float(j * cell_size + score_bar_height));
                window.draw(wall);
                break;
            default:
                if (game_state.field[j][i] == game_state.snake_length) {
                    float offset_x = head.getLocalBounds().width / 2;
                    float offset_y = head.getLocalBounds().height / 2;
                    head.setPosition(float(i * cell_size + offset_x), float(j * cell_size + score_bar_height + offset_y));
                    head.setOrigin(offset_x, offset_y);
                    switch (game_state.snake_direction) {
                    case SNAKE_DIRECTION_UP:
                        head.setRotation(0);
                        break;
                    case SNAKE_DIRECTION_RIGHT:
                        head.setRotation(90);
                        break;
                    case SNAKE_DIRECTION_DOWN:
                        head.setRotation(180);
                        break;
                    case SNAKE_DIRECTION_LEFT:
                        head.setRotation(-90);
                        break;
                    }
                    window.draw(head);
                }
                else {
                    snake.setPosition(float(i * cell_size), float(j * cell_size + score_bar_height));
                    window.draw(snake);
                }
            }
        }
    }
}

void draw_score_bar(sf::RenderWindow& window) //рисуем счетчик очков и прописываем логику счетчику
{
    window.draw(text_title);

    text_score.setString("Score: " + std::to_string(game_state.score));
    text_score.setPosition(window_width - text_score.getLocalBounds().width - 20, 10);
    window.draw(text_score);

    for (int i = 0; i < lives; i++) {
        life.setPosition((window_width - LIVES * 48) / 2 + i * 48, (score_bar_height - 48) / 2);
        window.draw(life);
    }
}

void draw_main_menu(sf::RenderWindow& window) //рисуем главное меню
{
    float const menu_padding_horizontal = 40; //задаем размеры
    float const menu_padding_vertical = 30;
    float const menu_item_interval = 20;

    float menu_item_max_width = 0;
    float current_menu_item_offset_y = 0;
    for (int i = 0; i < text_main_menu_items.size(); i++) { //провер€ем когда и какой элемент главного меню рисовать
        if (main_menu_items.at(i) == MENU_ITEM_START) {
            if (!game_over && game_started) {
                text_main_menu_items.at(i).setString(MENU_ITEM_RESUME);
            }
            else {
                text_main_menu_items.at(i).setString(MENU_ITEM_START);
            }
        }
        text_main_menu_items.at(i).setPosition(0, current_menu_item_offset_y);
        text_main_menu_items.at(i).setFillColor(current_main_menu_item_index == i ? sf::Color(224, 224, 224) : sf::Color(128, 128, 128));
        current_menu_item_offset_y += text_main_menu_items.at(i).getLocalBounds().height + menu_item_interval;
        menu_item_max_width = std::max(menu_item_max_width, text_main_menu_items.at(i).getLocalBounds().width);
    }

    float const menu_width = menu_item_max_width + menu_padding_horizontal * 2; //размеры
    float const menu_height = current_menu_item_offset_y - menu_item_interval + menu_padding_vertical * 2;

    float const menu_position_x = (window_width - menu_width) / 2; //положение на экране
    float const menu_position_y = (window_height - menu_height) / 2;

    sf::RectangleShape menu_rect(sf::Vector2f(menu_width, menu_height));
    menu_rect.setPosition(menu_position_x, menu_position_y);
    menu_rect.setFillColor(sf::Color(0, 0, 0, 224));
    window.draw(menu_rect);

    for (int i = 0; i < text_main_menu_items.size(); i++) {
        text_main_menu_items.at(i).move(menu_position_x + menu_padding_horizontal, menu_position_y + menu_padding_vertical);
        window.draw(text_main_menu_items.at(i));
    }
}

void draw_settings_menu(sf::RenderWindow& window) //рисуем меню настроек (задаем логику похожую на главное меню)
{
    float const menu_padding_horizontal = 40;
    float const menu_padding_vertical = 30;
    float const menu_item_interval = 20;

    float menu_item_max_width = 0;
    float current_menu_item_offset_y = 0;
    for (int i = 0; i < text_settings_menu_items.size(); i++) {
        if (settings_menu_items.at(i) == MENU_ITEM_VOLUME) {
            text_settings_menu_items.at(i).setString(settings_menu_items.at(i) + ": " + std::to_string(settings_volume));
        }
        text_settings_menu_items.at(i).setPosition(0, current_menu_item_offset_y);
        text_settings_menu_items.at(i).setFillColor(current_settings_menu_item_index == i ? sf::Color(224, 224, 224) : sf::Color(128, 128, 128));
        current_menu_item_offset_y += text_settings_menu_items.at(i).getLocalBounds().height + menu_item_interval;
        menu_item_max_width = std::max(menu_item_max_width, text_settings_menu_items.at(i).getLocalBounds().width);
    }

    float const menu_width = menu_item_max_width + menu_padding_horizontal * 2;
    float const menu_height = current_menu_item_offset_y - menu_item_interval + menu_padding_vertical * 2;

    float const menu_position_x = (window_width - menu_width) / 2;
    float const menu_position_y = (window_height - menu_height) / 2;

    sf::RectangleShape menu_rect(sf::Vector2f(menu_width, menu_height));
    menu_rect.setPosition(menu_position_x, menu_position_y);
    menu_rect.setFillColor(sf::Color(0, 0, 0, 224));
    window.draw(menu_rect);

    for (int i = 0; i < text_settings_menu_items.size(); i++) {
        text_settings_menu_items.at(i).move(menu_position_x + menu_padding_horizontal, menu_position_y + menu_padding_vertical);
        window.draw(text_settings_menu_items.at(i));
    }
}

void grow_snake() //змейка растет, увеличиваетс€
{
    for (int j = 0; j < field_size_y; j++) {
        for (int i = 0; i < field_size_x; i++) {
            if (game_state.field[j][i] > FIELD_CELL_TYPE_NONE) {
                game_state.field[j][i]++;
            }
        }
    }
}

void make_move() //перемещаем змейку на одну клетку (считываем направление и толкаем змейку)
{
    game_last_states.push_back(game_state);
    if (game_last_states.size() > 10) {
        game_last_states.erase(game_last_states.begin());
    }
    switch (game_state.snake_direction) {
    case SNAKE_DIRECTION_UP:
        game_state.snake_position_y--;
        if (game_state.snake_position_y < 0) {
            game_state.snake_position_y = field_size_y - 1;
        }
        break;
    case SNAKE_DIRECTION_RIGHT:
        game_state.snake_position_x++;
        if (game_state.snake_position_x > field_size_x - 1) {
            game_state.snake_position_x = 0;
        }
        break;
    case SNAKE_DIRECTION_DOWN:
        game_state.snake_position_y++;
        if (game_state.snake_position_y > field_size_y - 1) {
            game_state.snake_position_y = 0;
        }
        break;
    case SNAKE_DIRECTION_LEFT:
        game_state.snake_position_x--;
        if (game_state.snake_position_x < 0) {
            game_state.snake_position_x = field_size_x - 1;
        }
        break;
    }

    if (game_state.field[game_state.snake_position_y][game_state.snake_position_x] != FIELD_CELL_TYPE_NONE) {
        switch (game_state.field[game_state.snake_position_y][game_state.snake_position_x]) {
        case FIELD_CELL_TYPE_APPLE: //если она съела €блоко
            sound_ate_apple.play();
            game_state.snake_length++;
            game_state.score++;
            grow_snake();
            add_apple();
            break;
        case FIELD_CELL_TYPE_WALL: //если врезалась в стенку
            sound_died_against_the_wall.play();
            snake_died();
            break;
        default:
            if (game_state.field[game_state.snake_position_y][game_state.snake_position_x] > 1) { //если она съела саму себ€
                sound_ate_himself.play();
                snake_died();
            }
        }
    }

    if (!game_over && !rollback) { //убираем пустые клетки
        for (int j = 0; j < field_size_y; j++) {
            for (int i = 0; i < field_size_x; i++) {
                if (game_state.field[j][i] > FIELD_CELL_TYPE_NONE) {
                    game_state.field[j][i]--;
                }
            }
        }
        game_state.field[game_state.snake_position_y][game_state.snake_position_x] = game_state.snake_length;
    }
}

int main() //главна€ функци€
{
    init_game();

    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Growing hair", sf::Style::Close); //рисуем окно с игрой
    window.setIcon(image.getSize().x, image.getSize().y, image.getPixelsPtr());
    std::vector<int> snake_direction_queue;

    while (window.isOpen()) //делаем невозможным играть со свернутым окном
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) //закрываем окно
                window.close();
            if (event.type == sf::Event::KeyPressed) { //создаем клавиши управлени€
                if (game_paused) {
                    if (game_over_timeout == 0) {
                        if (current_menu == MENU_MAIN) { //дл€ главного меню
                            switch (event.key.code) {
                            case sf::Keyboard::Up:
                                sound_menu_navigate.play();
                                current_main_menu_item_index--;
                                if (current_main_menu_item_index < 0) {
                                    current_main_menu_item_index = text_main_menu_items.size() - 1;
                                }
                                break;
                            case sf::Keyboard::Down:
                                sound_menu_navigate.play();
                                current_main_menu_item_index++;
                                if (current_main_menu_item_index > text_main_menu_items.size() - 1) {
                                    current_main_menu_item_index = 0;
                                }
                                break;
                            case sf::Keyboard::Enter:
                                sound_menu_navigate.play();
                                if (main_menu_items.at(current_main_menu_item_index) == MENU_ITEM_START) {
                                    if (!game_over && game_started) {
                                        game_paused = false;
                                    }
                                    else {
                                        start_game();
                                    }
                                }
                                if (main_menu_items.at(current_main_menu_item_index) == MENU_ITEM_SETTINGS) {
                                    current_menu = MENU_SETTINGS;
                                    current_settings_menu_item_index = 0;
                                }
                                if (main_menu_items.at(current_main_menu_item_index) == MENU_ITEM_QUIT) {
                                    window.close();
                                }
                                break;
                            case sf::Keyboard::Escape:
                                sound_menu_navigate.play();
                                if (!game_over && game_started) {
                                    game_paused = false;
                                }
                                break;
                            }
                        }
                        else if (current_menu == MENU_SETTINGS) { //дл€ настроек
                            switch (event.key.code) {
                            case sf::Keyboard::Up:
                                sound_menu_navigate.play();
                                current_settings_menu_item_index--;
                                if (current_settings_menu_item_index < 0) {
                                    current_settings_menu_item_index = text_settings_menu_items.size() - 1;
                                }
                                break;
                            case sf::Keyboard::Down:
                                sound_menu_navigate.play();
                                current_settings_menu_item_index++;
                                if (current_settings_menu_item_index > text_settings_menu_items.size() - 1) {
                                    current_settings_menu_item_index = 0;
                                }
                                break;
                            case sf::Keyboard::Left:
                                if (settings_menu_items.at(current_settings_menu_item_index) == MENU_ITEM_VOLUME) {
                                    if (settings_volume > 0) {
                                        settings_volume -= 5;
                                        set_volume();
                                        sound_menu_navigate.play();
                                    }
                                }
                                break;
                            case sf::Keyboard::Right:
                                if (settings_menu_items.at(current_settings_menu_item_index) == MENU_ITEM_VOLUME) {
                                    if (settings_volume < 100) {
                                        settings_volume += 5;
                                        set_volume();
                                        sound_menu_navigate.play();
                                    }
                                }
                                break;
                            case sf::Keyboard::Enter:
                                sound_menu_navigate.play();
                                if (settings_menu_items.at(current_settings_menu_item_index) == MENU_ITEM_BACK) {
                                    current_menu = MENU_MAIN;
                                }
                                break;
                            case sf::Keyboard::Escape:
                                sound_menu_navigate.play();
                                current_menu = MENU_MAIN;
                                break;
                            }
                        }
                    }
                    else {
                        game_over_timeout = 0;
                    }
                }
                else { //дл€ игры
                    int snake_direction_last = snake_direction_queue.empty() ? game_state.snake_direction : snake_direction_queue.at(0);
                    switch (event.key.code) {
                    case sf::Keyboard::Up:
                        if (snake_direction_last != SNAKE_DIRECTION_UP && snake_direction_last != SNAKE_DIRECTION_DOWN) {
                            if (snake_direction_queue.size() < 2) {
                                snake_direction_queue.insert(snake_direction_queue.begin(), SNAKE_DIRECTION_UP);
                            }
                        }
                        break;
                    case sf::Keyboard::Right:
                        if (snake_direction_last != SNAKE_DIRECTION_RIGHT && snake_direction_last != SNAKE_DIRECTION_LEFT) {
                            if (snake_direction_queue.size() < 2) {
                                snake_direction_queue.insert(snake_direction_queue.begin(), SNAKE_DIRECTION_RIGHT);
                            }
                        }
                        break;
                    case sf::Keyboard::Down:
                        if (snake_direction_last != SNAKE_DIRECTION_DOWN && snake_direction_last != SNAKE_DIRECTION_UP) {
                            if (snake_direction_queue.size() < 2) {
                                snake_direction_queue.insert(snake_direction_queue.begin(), SNAKE_DIRECTION_DOWN);
                            }
                        }
                        break;
                    case sf::Keyboard::Left:
                        if (snake_direction_last != SNAKE_DIRECTION_LEFT && snake_direction_last != SNAKE_DIRECTION_RIGHT) {
                            if (snake_direction_queue.size() < 2) {
                                snake_direction_queue.insert(snake_direction_queue.begin(), SNAKE_DIRECTION_LEFT);
                            }
                        }
                        break;
                    case sf::Keyboard::Escape:
                        game_paused = true;
                        break;
                    }
                }
            }
        }
        if (!snake_direction_queue.empty()) {
            game_state.snake_direction = snake_direction_queue.back();
            snake_direction_queue.pop_back();
        }

        if (!game_paused) {
            if (!rollback) {
                make_move();
            }
            else {
                if (!game_last_states.empty()) {
                    game_state = game_last_states.back();
                    game_last_states.pop_back();
                }
                else {
                    rollback = false;
                }
            }
        }

        window.clear(sf::Color(183, 212, 168));

        draw_field(window);
        draw_score_bar(window);

        if (game_over) { //конец игры
            window.draw(text_game_over);
            if (game_over_timeout > 0) {
                game_over_timeout--;
            }
        }

        if (game_paused && game_over_timeout == 0) {
            switch (current_menu) {
            case MENU_MAIN:
                draw_main_menu(window);
                break;
            case MENU_SETTINGS:
                draw_settings_menu(window);
                break;
            }
        }

        window.display();

        sf::sleep(sf::milliseconds(100));
    }

    return 0;
}

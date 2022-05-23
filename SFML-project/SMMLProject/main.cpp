#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include "level.h"
#include "view.h"

using namespace sf;
using namespace std;

class Bullet
{
protected:
    float cur_pos_X = 0, cur_pos_Y = 0, pos_X = -10, pos_Y = -10;
    float time;
    float distance = 0;//это расстояние от объекта до тыка курсора
    bool aLife = false;

public:
    Image bullet_img;
    Texture bullet_texture;
    Sprite bullet_sprite;
    vector<Object> obj;//вектор объектов карты

    Bullet()
    {
        bullet_img.loadFromFile("images/bullet-1.png");
        bullet_texture.loadFromImage(bullet_img);
        bullet_sprite.setTexture(bullet_texture);      
    }

    bool get_aLife() { return aLife; }

    float get_posX() { return pos_X; }

    float get_posY() { return pos_Y; }

    void create(short tempX, short tempY, short posX, short posY)
    {
        aLife = true;
        pos_X = posX;
        pos_Y = posY;
        cur_pos_X = tempX;
        cur_pos_Y = tempY;
        bullet_sprite.setPosition(cur_pos_X, cur_pos_Y);
    }

    void projectile_flight(float time)
    {
        this->time = time;
        distance = sqrt((cur_pos_X - pos_X) * (cur_pos_X - pos_X) + (cur_pos_Y - pos_Y) * (cur_pos_Y - pos_Y));
        if (distance > 2) {//этим условием убираем дергание во время конечной позиции спрайта
            pos_X +=  time * (cur_pos_X - pos_X) / distance;//идем по иксу с помощью вектора нормали
            pos_Y +=  time * (cur_pos_Y - pos_Y) / distance;//идем по игреку так же
            bullet_sprite.setPosition(pos_X, pos_Y);
        }
        else
        {
            pos_X = 0;
            pos_Y = 0; 
            aLife = false;
            bullet_img.createMaskFromColor(Color(0, 0, 0));
        }
    }

    FloatRect getRect() { return FloatRect(pos_X, pos_Y, 12, 12); }
};

class Location
{
protected:
    float dx, dy, x, y;
public:
    float get_cord_X() { return x; }
    float get_cord_Y() { return y; }
};

class Point : public Location
{
private:
    float speed;
    
public:
    void set_speed(float Speed) { speed = Speed; }
    float get_speed() { return speed; }
};

class Ship : public Point
{
protected:
    float current_frame = 0, w, h;
    short dir;
public:
    Image ship_img;
    Texture ship_texture;
    Sprite ship_sprite;
    string direction_ship;
    vector<Object> obj;//вектор объектов карты
    
    Ship(string file_name, float X, float Y, short W, short H)
    {
        w = W, h = H, x = X, y = Y;
        string File = file_name;
        ship_img.loadFromFile("images/" + File);
        ship_texture.loadFromImage(ship_img);
        ship_sprite.setTexture(ship_texture);
        ship_sprite.setTextureRect(IntRect(0, 0, 120, 90));
        ship_sprite.setPosition(x, y);
        ship_sprite.setOrigin(w / 2, h / 2 + 20);
    }
    FloatRect getRect() { return FloatRect(x, y, w, h); }    
    virtual void update(float time) = 0;
};

class Main_Ship : public Ship
{
public:

    Main_Ship(string file_name, TileMap &levl, float X, float Y, short W, short H) : Ship(file_name, X, Y, W, H) { obj = levl.getAllObjects(); }
    void update(float time);
    void control();
    void check_collision_with_map(float Dx, float Dy);    
};

class Enemy_Ship : public Ship
{
public:
    Enemy_Ship(string file_name, TileMap& levl, float X, float Y, short W, short H) : Ship(file_name, X, Y, W, H) {}
    void update(float time)
    {
        x += dx * time;
        ship_sprite.setPosition(x + w / 2, y + h / 2); //задаем позицию спрайта в место его центра  
    }
};

int main()
{
    int tempX = 0;//временная коорд Х.Снимаем ее после нажатия прав клав мыши
    int tempY = 0;//коорд Y
    bool flag_draw_bullet = false;
    
    RenderWindow window(VideoMode(1200, 600), "okno");
    view.reset(FloatRect(0, 0, 1200, 600));
    TileMap lvl;
    Clock clock;
    lvl.load("images/first_map2.tmx");

    Object player = lvl.getObject("player");//объект игрока на нашей карте.задаем координаты игроку в начале при помощи него
    Main_Ship ship_first("ship-main.png", lvl, player.rect.left, player.rect.top, 120, 90);
    Ship* arr[5];
    arr[0] = &ship_first;

    list <Ship*> enemies_ships;
    vector <Object> enemies_get_obj = lvl.getAllObjects();    

    Bullet bullet;

    for (int i = 0; i < enemies_get_obj.size(); i++)
        if (enemies_get_obj[i].name == "enemy")
            enemies_ships.push_back(new Enemy_Ship("ship-2.png", lvl, enemies_get_obj[i].rect.left, enemies_get_obj[i].rect.top, 52, 155));
    list<Ship*>::iterator it;

    while (window.isOpen())
    {
        Event event;
        float time = clock.getElapsedTime().asMicroseconds();
        clock.restart();
        time /= 800;
        Vector2i pixelPos = Mouse::getPosition(window);//забираем коорд курсора
        Vector2f pos = window.mapPixelToCoords(pixelPos);//переводим их в игровые (уходим от коорд окна)
        //cout << pos.x << " " << pos.y << endl;

        arr[0]->update(time);

        window.clear();
        window.setView(view);
        window.draw(lvl);   

        for (list <Ship*>::iterator iter = enemies_ships.begin(); iter != enemies_ships.end(); iter++)
        {
            (*iter)->update(time);
            window.draw((*iter)->ship_sprite); //cout << (*iter)->get_cord_X() << " " << (*iter)->get_cord_Y() << endl;

            for (int i = 0; i < enemies_ships.size(); i++)

                if ((bullet.get_posX() >= (*iter)->get_cord_X()) && (bullet.get_posX() <= (*iter)->get_cord_X() + 120) && (bullet.get_posY() >= (*iter)->get_cord_Y() + 42) && (bullet.get_posY() <= (*iter)->get_cord_Y() + 67)) //если этот объект столкнулся с пулей,
                {
                    iter = enemies_ships.erase(iter);
                }
        }
            
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed || Keyboard::isKeyPressed(Keyboard::Escape))
                window.close();
            if (event.type == Event::MouseButtonPressed)
                if (event.key.code == Mouse::Left)
                {
                    bullet.create(pos.x, pos.y, arr[0]->get_cord_X(), arr[0]->get_cord_Y());
                }                    
        }
        if (bullet.get_aLife())
        {
            bullet.projectile_flight(time);
            window.draw(bullet.bullet_sprite);
        }

        window.draw(arr[0]->ship_sprite);
        
        
        window.display();
    }
}

void Main_Ship::update(float time)
{
    control();
    switch (dir)
    {
    case 0:
        dx = get_speed();
        dy = 0;
        break;
    case 1:
        dx = -get_speed();
        dy = 0;
        break;
    case 2:
        dx = 0;
        dy = get_speed();
        break;
    case 3:
        dx = 0;
        dy = -get_speed();
        break;
    }
    x += dx * time;
    check_collision_with_map(dx, 0);
    y += dy * time;
    check_collision_with_map(0, dy);
    set_speed(0);
    get_coord_for_view(get_cord_X(), get_cord_Y());
    ship_sprite.setPosition(x, y);
}

void Main_Ship::check_collision_with_map(float Dx, float Dy)
{
    for (int i = 0; i < obj.size(); i++)//проходимся по объектам
        if (getRect().intersects(obj[i].rect))//проверяем пересечение игрока с объектом
        {
            if (obj[i].name == "solid")//если встретили препятствие
            {
                if (Dy > 0) { y = obj[i].rect.top - h;  dy = 0; }
                if (Dy < 0) { y = obj[i].rect.top + obj[i].rect.height;   dy = 0; }
                if (Dx > 0) { x = obj[i].rect.left - w; dx = 0; }
                if (Dx < 0) { x = obj[i].rect.left + obj[i].rect.width; dx = 0;}
            }
        }
}

void Main_Ship::control()
{
    if (Keyboard::isKeyPressed(Keyboard::Left))
    {
        dir = 1;
        set_speed(0.1);
        ship_sprite.setTextureRect(IntRect(0, 0, 115, 89));
    }
    else if (Keyboard::isKeyPressed(Keyboard::Right))
    {
        dir = 0;
        set_speed(0.1);
        ship_sprite.setTextureRect(IntRect(120, 0, 115, 89));
    }
    else if (Keyboard::isKeyPressed(Keyboard::Up))
    {
        dir = 3;
        set_speed(0.1);
    }
    else if (Keyboard::isKeyPressed(Keyboard::Down))
    {
        dir = 2;
        set_speed(0.1);
    }
}
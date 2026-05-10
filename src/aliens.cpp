#include "aliens.hpp"
#include "raylib.h"

Texture2D Alien::alienImages[3] = {};
Alien::Alien(int type , Vector2 position){
    this -> type =type;
    this -> position = position;
if (alienImages[type -1].id == 0){
    // OLD CODE:
    // switch(type){
    //     case 1:
    //     alienImages[0] = LoadTexture("assets/sprites/e1.png");
    //     break;
    //      case 2:
    //     alienImages[1] = LoadTexture("assets/sprites/e2.png");
    //     break;
    //      case 3:
    //     alienImages[2] = LoadTexture("assets/sprites/e3i.png");
    //     break;
    //    default:
    //     alienImages[0]= LoadTexture("assets/sprites/e1i.png");
    //     break;
    // }
    // NEW CODE: Load image, resize it by half, and then create texture
    Image img;
    switch(type){
        case 1:
        img = LoadImage("assets/sprites/e1.png");
        ImageResize(&img, img.width / 2, img.height / 2);
        alienImages[0] = LoadTextureFromImage(img);
        UnloadImage(img);
        break;
         case 2:
        img = LoadImage("assets/sprites/e2.png");
        ImageResize(&img, img.width / 2, img.height / 2);
        alienImages[1] = LoadTextureFromImage(img);
        UnloadImage(img);
        break;
         case 3:
        img = LoadImage("assets/sprites/e3i.png");
        ImageResize(&img, img.width / 2, img.height / 2);
        alienImages[2] = LoadTextureFromImage(img);
        UnloadImage(img);
        break;
       default:
        img = LoadImage("assets/sprites/e1i.png");
        ImageResize(&img, img.width / 2, img.height / 2);
        alienImages[0]= LoadTextureFromImage(img);
        UnloadImage(img);
        break;
    }
    }

}
void Alien::Draw()
{
    DrawTextureV(alienImages[type -1] , position , WHITE );

}

int Alien::GetType(){
    return type ;
}

void Alien::unloadImages(){
    for (int i = 0; i < 4 ; i++){

    UnloadTexture(alienImages[i]);
}
}


void Alien::Update(int direction){
    position.x += direction;

}
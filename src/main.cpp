#include "lvgl.h"
#include "lv_conf.h"
#include <stdio.h>
//#include "colorbar.h"


#define LV_USE_FS_STDIO 1 // Pour les systèmes de fichiers standard (comme sur PC)
#define LV_USE_FS_FATFS 1
#define LV_USE_TJPGD 1 // Pour les images JPG
#define LV_USE_PNG 1  

typedef FILE * my_file_t;

 
static lv_obj_t * slider_label;
static lv_obj_t * bleu;
static lv_obj_t * vert;
static lv_obj_t * rouge;
lv_obj_t * AUTO = NULL;
bool lance = false;

static lv_timer_t * auto_color_timer = NULL;



 /*const lv_image_dsc_t colorbar = {
    {  // Initialisation du champ `header`
        LV_IMAGE_HEADER_MAGIC,  // magic
        48,                     // w
        468,                    // h
        LV_COLOR_FORMAT_RGB565  // cf
    },
    22464 * 2,                  // data_size
    colorbar_map,               // data
    NULL                        // reserved
};*/


// Pointeur vers le fichier réel, sera géré par votre système de fichiers sous-jacent
typedef FILE * my_file_t; // Ou SdFile * pour Arduino SD

// Fonctions de rappel pour le système de fichiers LVGL
static void * my_open_cb(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode)
{
    // Convertir le mode LVGL en mode de système de fichiers réel (ex: "r", "w")
    const char * fs_mode = "";
    if (mode == LV_FS_MODE_RD) fs_mode = "rb"; // Lecture binaire
    else if (mode == LV_FS_MODE_WR) fs_mode = "wb"; // Écriture binaire
    else if (mode == (LV_FS_MODE_RD | LV_FS_MODE_WR)) fs_mode = "rb+"; // Lecture/écriture binaire
    else return NULL; // Mode non supporté

    my_file_t f = fopen(path, fs_mode); // Utilisez la fonction d'ouverture de votre système de fichiers
    return (void *)f;
}

static lv_fs_res_t my_close_cb(lv_fs_drv_t * drv, void * file_p)
{
    fclose((my_file_t)file_p); // Utilisez la fonction de fermeture de votre système de fichiers
    return LV_FS_RES_OK;
}

static lv_fs_res_t my_read_cb(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
    *br = fread(buf, 1, btr, (my_file_t)file_p); // Utilisez la fonction de lecture de votre système de fichiers
    return LV_FS_RES_OK;
}

static lv_fs_res_t my_seek_cb(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence)
{
    fseek((my_file_t)file_p, pos, (int)whence); // Utilisez la fonction de seek de votre système de fichiers
    return LV_FS_RES_OK;
}

static lv_fs_res_t my_tell_cb(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
    *pos_p = ftell((my_file_t)file_p); // Utilisez la fonction de tell de votre système de fichiers
    return LV_FS_RES_OK;
}

// Initialisation du pilote de système de fichiers LVGL
void lvgl_fs_init(void)
{
    static lv_fs_drv_t fs_drv;
    lv_fs_drv_init(&fs_drv); // Initialise le pilote avec les valeurs par défaut
    fs_drv.letter = 'U'; // Attribuez une lettre de lecteur (ex: 'S' pour SD)
    fs_drv.open_cb = my_open_cb;
    fs_drv.close_cb = my_close_cb;
    fs_drv.read_cb = my_read_cb;
    fs_drv.seek_cb = my_seek_cb;
    fs_drv.tell_cb = my_tell_cb;
    // Les callbacks pour les répertoires (dir_open, dir_read, dir_close) sont facultatifs si vous n'avez pas besoin de lister les fichiers
    lv_fs_drv_register(&fs_drv); // Enregistre le pilote avec LVGL
}

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}


void testLvgl()
{
  // Initialisations générales
  /*lv_obj_t * label;

  lv_obj_t * btn2 = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
  lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
  lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_height(btn2, LV_SIZE_CONTENT);

  label = lv_label_create(btn2);
  lv_label_set_text(label, "Toggle");
  lv_obj_center(label);*/
}

#ifdef ARDUINO

//def les pin rgb
#define R  10  
#define G  9 
#define B  11 

#include "lvglDrivers.h"

// à décommenter pour tester la démo
// #include "demos/lv_demos.h"
lv_obj_t * slider = NULL;

int A = -255 / 255;



static int redled(int sli)
{
  int red;
  int slideb;
  int Y;
  

  //int rem = sli % 255; // se remet à 0 pour eviter de depasser 255
  //float ratio = rem / 255.0; 


  if ((sli>=0)&&(sli<1*255)) // g monte r reste
  {
    red = 255;
  }
  else if ((sli>=1*255)&&(sli<2*255)) //r descend g reste
  {
    slideb = 1*255;
    Y = 255;

    red = A*(sli-slideb)+Y;
  }
  else if ((sli>=4*255)&&(sli<5*255)) //r monte b reste
  {
    slideb = 4*255;
    Y = 0;
    
    red = -A*(sli-slideb)+Y;
  }
  else if ((sli>=5*255)&&(sli<6*255)) // b descend r reste
  {
    red = 255;
  }
  else {
    red = 0;  // Valeur par défaut si aucune condition ne correspond
  }

 return red;

}


static int greenled(int sli)
{
  int gre;
  int slideb;
  int Y;

  if ((sli>=0)&&(sli<1*255)) // g monte 
  {
    slideb = 0;
    Y = 0;
    
    gre = -A*(sli-slideb)+Y;
  }
  else if ((sli>=1*255)&&(sli<2*255))// r descend g reste 
  {
    gre = 255;

  }
  else if ((sli>=2*255)&&(sli<3*255)) // b monte g reste
  {
    gre = 255;

  }
  else if ((sli>=3*255)&&(sli<4*255)) //g descend
  {
    slideb = 3*255;
    Y = 255;

    gre = A*(sli-slideb)+Y;

  }
  else {
    gre = 0;  // Valeur par défaut si aucune condition ne correspond
  }

  return gre;

}

static int blueled(int sli)
{

  int blue;
  int slideb;
  int Y;
  
  if ((sli>=2*255)&&(sli<3*255)) //b monte g reste
  {
    slideb = 2*255;
    Y = 0;

    blue = -A*(sli-slideb)+Y;

  }
  else if ((sli>=3*255)&&(sli<4*255)) // g descend b reste
  {
    blue = 255;

  }
  else if ((sli>=4*255)&&(sli<5*255)) // r monte b reste
  {
    blue = 255;

  }
  else if ((sli>=5*255)&&(sli<6*255)) // b descend
  {
    slideb = 5*255;
    Y = 255;

    blue = A*(sli-slideb)+Y;

  }
  else {
    blue = 0;  // Valeur par défaut si aucune condition ne correspond
  }
  

  return blue;

}

static int animation_step = 0; // Le compteur pour l'animation
static int plus = 0;






static void ledauto (lv_event_t * e){

lv_event_code_t bp = lv_event_get_code(e);
lv_obj_t * btn = (lv_obj_t *) lv_event_get_target(e);
 
 if (bp == LV_EVENT_CLICKED){

  

  lance = true;
  plus++;

  if (plus == 2) { 
    
    lance = false;
    plus = 0;
  }


  }
}




static void slider_event_cb(lv_event_t * e)
{
  lv_obj_t * slider = lv_event_get_target_obj(e);
  char buf[8];
  char r[8];
  char g[8];
  char b[8];

  lv_snprintf(buf, sizeof(buf), "%d%%", ((int)lv_slider_get_value(slider)));
  lv_label_set_text(slider_label, buf);
  lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);


  int red, blue, green;

  red = redled(lv_slider_get_value(slider));
  blue = blueled(lv_slider_get_value(slider));
  green = greenled(lv_slider_get_value(slider));


  lv_snprintf(r, sizeof(r), "%d%%", red);
  lv_snprintf(g, sizeof(g), "%d%%", green);
  lv_snprintf(b, sizeof(b), "%d%%", blue);

  lv_label_set_text(rouge, r);
  lv_label_set_text(vert, g);
  lv_label_set_text(bleu, b);
  
  lv_obj_align_to(rouge, slider, LV_ALIGN_OUT_BOTTOM_MID, -40, 30);
  lv_obj_align_to(vert, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
  lv_obj_align_to(bleu, slider, LV_ALIGN_OUT_BOTTOM_MID, 40, 30);
  //printf("R = %d, B = %d, G = %d\n", red, blue, green);

  analogWrite(R, red);    
  analogWrite(G, green);    
  analogWrite(B, blue); 

}





// variable lumino
/*static void ledrog (lv_event_t * e)
{
 lv_event_code_t bp = lv_event_get_code(e);
 lv_obj_t * btn = (lv_obj_t *) lv_event_get_target(e);
 
 if (bp == LV_EVENT_CLICKED){
  analogWrite(R, 250);    
  analogWrite(G, 0);    
  analogWrite(B, 0); 
 }
}

static void ledble (lv_event_t * e)
{
 lv_event_code_t bp = lv_event_get_code(e);
 lv_obj_t * btn = (lv_obj_t *) lv_event_get_target(e);
 
 if (bp == LV_EVENT_CLICKED){
  analogWrite(R, 0);    
  analogWrite(G, 0);    
  analogWrite(B, 250); 
 }
}

static void ledver (lv_event_t * e)
{
 lv_event_code_t bp = lv_event_get_code(e);
 lv_obj_t * btn = (lv_obj_t *) lv_event_get_target(e);
 
 if (bp == LV_EVENT_CLICKED){
  analogWrite(R, 0);    
  analogWrite(G, 250);    
  analogWrite(B, 0); 
 }
}

static void ledbla (lv_event_t * e)
{
 lv_event_code_t bp = lv_event_get_code(e);
 lv_obj_t * btn = (lv_obj_t *) lv_event_get_target(e);
 
 if (bp == LV_EVENT_CLICKED){
  analogWrite(R, 200);    
  analogWrite(G, 200);    
  analogWrite(B, 200); 
 }
}

static void ledja (lv_event_t * e)
{
 lv_event_code_t bp = lv_event_get_code(e);
 lv_obj_t * btn = (lv_obj_t *) lv_event_get_target(e);
 
 if (bp == LV_EVENT_CLICKED){
  analogWrite(R, 243);    
  analogWrite(G, 250);    
  analogWrite(B, 0); 
 }
}

static void ledor (lv_event_t * e)
{
 lv_event_code_t bp = lv_event_get_code(e);
 lv_obj_t * btn = (lv_obj_t *) lv_event_get_target(e);
 
 if (bp == LV_EVENT_CLICKED){
  analogWrite(R, 250);    
  analogWrite(G, 128);    
  analogWrite(B, 0); 
 }
}

static void ledmar (lv_event_t * e)
{
 lv_event_code_t bp = lv_event_get_code(e);
 lv_obj_t * btn = (lv_obj_t *) lv_event_get_target(e);
 
 if (bp == LV_EVENT_CLICKED){
  analogWrite(R, 115);    
  analogWrite(G, 53);    
  analogWrite(B, 7); 
 }
}

static void ledvio (lv_event_t * e)
{
 lv_event_code_t bp = lv_event_get_code(e);
 lv_obj_t * btn = (lv_obj_t *) lv_event_get_target(e);
 
 if (bp == LV_EVENT_CLICKED){
  analogWrite(R, 139);    
  analogWrite(G, 0);    
  analogWrite(B, 250); 
 }
}




static void ledros (lv_event_t * e)
{
 lv_event_code_t bp = lv_event_get_code(e);
 lv_obj_t * btn = (lv_obj_t *) lv_event_get_target(e);
 
 if (bp == LV_EVENT_CLICKED){
  analogWrite(R, 250);    
  analogWrite(G, 0);    
  analogWrite(B, 232); 
 }
}*/

lv_obj_t * btnrog = NULL;
lv_obj_t * btnble = NULL;
lv_obj_t * btnver = NULL;
lv_obj_t * btnbla = NULL;
lv_obj_t * btnja = NULL;
lv_obj_t * btnor = NULL;
lv_obj_t * btnmar = NULL;
lv_obj_t * btnros = NULL;
lv_obj_t * btnmov = NULL;
lv_obj_t * btnvio = NULL;
lv_obj_t * img_bg = NULL;
lv_obj_t * btn2 = NULL;


lv_obj_t * img2 = NULL;


static lv_style_t style_main;
static lv_style_t style_indicator;
static lv_style_t style_knob;
static lv_style_t style_pressed_color;

static void ledon (lv_event_t * e)
{
 lv_event_code_t bp = lv_event_get_code(e);
 lv_obj_t * btn = (lv_obj_t *) lv_event_get_target(e);
 
 if (bp == LV_EVENT_CLICKED){
  //BP COULEUR ROUGE

  lv_obj_t * label;
  

   //LV_IMG_DECLARE(colorbar);  // Déclaration de l'image

//BP LED AUTO

  AUTO = lv_button_create(lv_screen_active());
  lv_obj_align(AUTO, LV_ALIGN_CENTER, 0, -50);
  Serial.printf("align ok");
  lv_obj_remove_flag(AUTO, LV_OBJ_FLAG_PRESS_LOCK);

  label = lv_label_create(AUTO);
  lv_label_set_text(label, "Auto");
  Serial.printf("text ok");
  lv_obj_center(label);
  //testLvgl();
  lv_obj_add_event_cb(AUTO, ledauto, LV_EVENT_CLICKED, NULL);


    img_bg = lv_img_create(lv_screen_active());  // Crée l'objet image sur l'écran actif
    lv_img_set_src(img_bg, "U:/img/colorbar.bin");  // Associe l'image à l'objet

    /* Positionner l'image en fond */
    lv_obj_align(img_bg, LV_ALIGN_CENTER, 0, 0);  // Centrer l'image dans l'écran
    //lv_obj_set_style_opa(img_bg, LV_OPA_COVER, 0);  // Assurez-vous que l'image couvre entièrement l'arrière-plan

    img2 = lv_image_create(lv_screen_active());
   // lv_image_set_src(img2, LV_SYMBOL_OK "Accept");
    lv_obj_align_to(img2, img_bg, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    /* Créer un style pour l'indicateur */
    static lv_style_t style_indicator;
    lv_style_init(&style_indicator);
    lv_style_set_bg_color(&style_indicator, lv_palette_main(LV_PALETTE_CYAN));
    lv_style_set_radius(&style_indicator, LV_RADIUS_CIRCLE);

    /* Créer un style pour le bouton (knob) */
    static lv_style_t style_knob;
    lv_style_init(&style_knob);
    lv_style_set_bg_color(&style_knob, lv_palette_main(LV_PALETTE_CYAN));
    lv_style_set_radius(&style_knob, LV_RADIUS_CIRCLE);
    lv_style_set_border_width(&style_knob, 2);
    lv_style_set_border_color(&style_knob, lv_palette_darken(LV_PALETTE_CYAN, 2));
    lv_style_set_pad_all(&style_knob, 8);  // Rendre le knob plus grand

    /* Créer un slider */
    slider = lv_slider_create(lv_screen_active());  // Crée un slider
    lv_obj_remove_style_all(slider);  // Supprime les styles par défaut
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);



    /* Ajouter les styles au slider */
    lv_obj_add_style(slider, &style_indicator, LV_PART_INDICATOR);  // Applique le style à l'indicateur
    lv_obj_add_style(slider, &style_knob, LV_PART_KNOB);  // Applique le style au bouton (knob)

    /* Définir la plage du slider de 0 à 1000 */
    lv_slider_set_range(slider, 0, 1530);
    slider_label = lv_label_create(lv_screen_active());
    lv_label_set_text(slider_label, "0%");


    rouge = lv_label_create(lv_screen_active());
    vert = lv_label_create(lv_screen_active());
    bleu = lv_label_create(lv_screen_active());
    lv_label_set_text(rouge, "0%");
    lv_label_set_text(vert, "0%");
    lv_label_set_text(bleu, "0%");
    

    /* Positionner le slider au-dessus de l'image */
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, 0);

    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_align_to(rouge, slider, LV_ALIGN_OUT_BOTTOM_MID, -30, 30);
    lv_obj_align_to(vert, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
    lv_obj_align_to(bleu, slider, LV_ALIGN_OUT_BOTTOM_MID, 30, 30);


    /* Centrer le slider sur l'écran */
    lv_obj_center(slider);

  /*btnrog = lv_button_create(lv_screen_active());
  lv_obj_align(btnrog, LV_ALIGN_CENTER, -190, -100);
  Serial.printf("align ok");
  lv_obj_remove_flag(btnrog, LV_OBJ_FLAG_PRESS_LOCK);

  label = lv_label_create(btnrog);
  lv_label_set_text(label, "Rouge");
  Serial.printf("text ok");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnrog, ledrog, LV_EVENT_CLICKED, NULL);

//BP COULEUR BLEU

  btnble = lv_button_create(lv_screen_active());
  lv_obj_align(btnble, LV_ALIGN_CENTER, -190, -60);
  Serial.printf("align ok");
  lv_obj_remove_flag(btnble, LV_OBJ_FLAG_PRESS_LOCK);

  label = lv_label_create(btnble);
  lv_label_set_text(label, "Bleu");
  Serial.printf("text ok");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnble, ledble, LV_EVENT_CLICKED, NULL);

//BP COULEUR VERT

  btnver = lv_button_create(lv_screen_active());
  lv_obj_align(btnver, LV_ALIGN_CENTER, -190, -20);
  Serial.printf("align ok");
  lv_obj_remove_flag(btnver, LV_OBJ_FLAG_PRESS_LOCK);

  label = lv_label_create(btnver);
  lv_label_set_text(label, "Vert");
  Serial.printf("text ok");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnver, ledver, LV_EVENT_CLICKED, NULL);

//BP COULEUR BLANC

  btnbla = lv_button_create(lv_screen_active());
  lv_obj_align(btnbla, LV_ALIGN_CENTER, -190, 20);
  Serial.printf("align ok");
  lv_obj_remove_flag(btnbla, LV_OBJ_FLAG_PRESS_LOCK);

  label = lv_label_create(btnbla);
  lv_label_set_text(label, "Blanc");
  Serial.printf("text ok");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnbla, ledbla, LV_EVENT_CLICKED, NULL);

//BP COULEUR JAUNE

  btnja = lv_button_create(lv_screen_active());
  lv_obj_align(btnja, LV_ALIGN_CENTER, -190, 60);
  Serial.printf("align ok");
  lv_obj_remove_flag(btnja, LV_OBJ_FLAG_PRESS_LOCK);

  label = lv_label_create(btnja);
  lv_label_set_text(label, "Jaune");
  Serial.printf("text ok");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnja, ledja, LV_EVENT_CLICKED, NULL);

//BP COULEUR ORANGE

  btnor = lv_button_create(lv_screen_active());
  lv_obj_align(btnor, LV_ALIGN_CENTER, -190, 100);
  Serial.printf("align ok");
  lv_obj_remove_flag(btnor, LV_OBJ_FLAG_PRESS_LOCK);

  label = lv_label_create(btnor);
  lv_label_set_text(label, "Orange");
  Serial.printf("text ok");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnor, ledor, LV_EVENT_CLICKED, NULL);

//BP COULEUR MARRON

  btnmar = lv_button_create(lv_screen_active());
  lv_obj_align(btnmar, LV_ALIGN_CENTER, -95, -60);
  Serial.printf("align ok");
  lv_obj_remove_flag(btnmar, LV_OBJ_FLAG_PRESS_LOCK);

  label = lv_label_create(btnmar);
  lv_label_set_text(label, "Marron");
  Serial.printf("text ok");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnmar, ledmar, LV_EVENT_CLICKED, NULL);

//BP COULEUR ROSE

  btnros = lv_button_create(lv_screen_active());
  lv_obj_align(btnros, LV_ALIGN_CENTER, -95, -20);
  Serial.printf("align ok");
  lv_obj_remove_flag(btnros, LV_OBJ_FLAG_PRESS_LOCK);

  label = lv_label_create(btnros);
  lv_label_set_text(label, "Rose");
  Serial.printf("text ok");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnros, ledros, LV_EVENT_CLICKED, NULL);


//BP COULEUR VIOLET

  btnvio = lv_button_create(lv_screen_active());
  lv_obj_align(btnvio, LV_ALIGN_CENTER, -95, 60);
  Serial.printf("align ok");
  lv_obj_remove_flag(btnvio, LV_OBJ_FLAG_PRESS_LOCK);

  label = lv_label_create(btnvio);
  lv_label_set_text(label, "Violet");
  Serial.printf("text ok");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnvio, ledvio, LV_EVENT_CLICKED, NULL);
  */
 
 
 }
}

static void ledoff (lv_event_t * e)
{
 lv_event_code_t bp = lv_event_get_code(e);
 lv_obj_t * btn = (lv_obj_t *) lv_event_get_target(e);
 
 if (bp == LV_EVENT_CLICKED){

  lance = false;
  animation_step = 0;

  analogWrite(R, 0);    
  analogWrite(G, 0);    
  analogWrite(B, 0);

  lv_obj_del(btnrog);
        btnrog = NULL;
  lv_obj_del(btnble);
        btnble = NULL;
  lv_obj_del(btnver);
        btnver = NULL;
  lv_obj_del(btnbla);
        btnbla = NULL;
  lv_obj_del(btnja);
        btnja = NULL;
  lv_obj_del(btnor);
        btnor = NULL;
  lv_obj_del(btnmar);
        btnmar = NULL;
  lv_obj_del(btnros);
        btnros = NULL;
  lv_obj_del(img_bg);
        img_bg = NULL;
  lv_obj_del(btnvio);
        btnvio = NULL;       
  lv_obj_del(slider_label);
        slider_label = NULL;  
  lv_obj_del(slider);
        slider = NULL; 
  lv_obj_del(img2);
        img2 = NULL; 
  lv_obj_del(rouge);
        rouge = NULL;      
  lv_obj_del(vert);
        vert = NULL;
  lv_obj_del(bleu);
        bleu = NULL;
  lv_obj_del(AUTO);
        AUTO = NULL;
         

 }
}



void mySetup()
{

  //pin entre ou sorti
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);

  // à décommenter pour tester la démo
  // lv_demo_widgets();

  // Initialisations générales
  lv_obj_t * label;

//BP LED ON

  lv_obj_t * btn1 = lv_button_create(lv_screen_active());
  lv_obj_align(btn1, LV_ALIGN_CENTER, 210, -20);
  Serial.printf("align ok");
  lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

  label = lv_label_create(btn1);
  lv_label_set_text(label, "On");
  Serial.printf("text ok");
  lv_obj_center(label);
  //testLvgl();
  lv_obj_add_event_cb(btn1, ledon, LV_EVENT_CLICKED, NULL);

//BP LED OFF

  btn2 = lv_button_create(lv_screen_active());
  lv_obj_align(btn2, LV_ALIGN_CENTER, 210, 20);
  Serial.printf("align ok");
  lv_obj_remove_flag(btn2, LV_OBJ_FLAG_PRESS_LOCK);

  label = lv_label_create(btn2);
  lv_label_set_text(label, "Off");
  Serial.printf("text ok");
  lv_obj_center(label);
  lv_obj_add_event_cb(btn2, ledoff, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t * titre = lv_label_create(lv_screen_active());
  lv_label_set_text(titre, "CONTROLEUR LED");
  lv_obj_align(titre, LV_ALIGN_CENTER, 0, -100);








}

void loop()
{
  
  if(lance == true){
  
  

   // Incrémente le pas de l'animation
     // Si on atteint la fin, on réinitialise
      animation_step++;
    if (animation_step >= 1530){
      animation_step = 0;
    }

  int autosli = animation_step;
  
  char buf[8];
  char r[8];
  char g[8];
  char b[8];

  lv_snprintf(buf, sizeof(buf), "%d%%", autosli);
  lv_label_set_text(slider_label, buf);
  lv_obj_align_to(slider_label, AUTO, LV_ALIGN_OUT_BOTTOM_MID, 0, 50);


  int red, blue, green;

  red = redled(autosli);
  blue = blueled(autosli);
  green = greenled(autosli);


  lv_snprintf(r, sizeof(r), "%d%%", red);
  lv_snprintf(g, sizeof(g), "%d%%", green);
  lv_snprintf(b, sizeof(b), "%d%%", blue);

  lv_label_set_text(rouge, r);
  lv_label_set_text(vert, g);
  lv_label_set_text(bleu, b);
  
  lv_obj_align_to(rouge, AUTO, LV_ALIGN_OUT_BOTTOM_MID, -40, 70);
  lv_obj_align_to(vert, AUTO, LV_ALIGN_OUT_BOTTOM_MID, 0, 70);
  lv_obj_align_to(bleu, AUTO, LV_ALIGN_OUT_BOTTOM_MID, 40, 70);
  //printf("R = %d, B = %d, G = %d\n", red, blue, green);

  analogWrite(R, red);    
  analogWrite(G, green);    
  analogWrite(B, blue); 

  delay(20);

  }
  
    lv_timer_handler();
  

// si je veux que la bande led change en fonction de l'ecran je dois recup la valeur donner mettre dans une autre variable et potentiellement metre en nombre 

  // Inactif (pour mise en veille du processeur)
}

void myTask(void *pvParameters)
{
  // Init
  TickType_t xLastWakeTime;
  // Lecture du nombre de ticks quand la tâche débute
  xLastWakeTime = xTaskGetTickCount();
  while (1)
  {
    // Loop

    // Endort la tâche pendant le temps restant par rapport au réveil,
    // ici 200ms, donc la tâche s'effectue toutes les 200ms
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(200)); // toutes les 200 ms
  }
}

#else

#include "lvgl.h"
#include "app_hal.h"
#include <cstdio>

int main(void)
{
  printf("LVGL Simulator\n");
  fflush(stdout);

  lv_init();
  hal_setup();

  testLvgl();

  hal_loop();
  return 0;
}

#endif

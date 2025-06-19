// Inclusion de la bibliothèque principale LVGL (interface graphique)
#include "lvgl.h"

// Inclusion du fichier de configuration LVGL (options activées, tailles, couleurs, etc.)
#include "lv_conf.h"

// Inclusion de la bibliothèque standard C pour la gestion des fichiers, entrées/sorties
#include <stdio.h>

//#include "colorbar.h"  // (Commenté) - Sert probablement à inclure une image pour l’arrière-plan du slider

// Activation des modules de fichiers et formats image dans LVGL
#define LV_USE_FS_STDIO 1   // Utilisation du système de fichiers standard (stdio)
#define LV_USE_FS_FATFS 1   // Support du système de fichiers FatFS (ex : carte SD)
#define LV_USE_TJPGD 1      // Support du format image JPEG
#define LV_USE_PNG 1        // Support du format image PNG

// Définition d'un alias pour simplifier l'utilisation de pointeurs de fichiers
typedef FILE * my_file_t;

// Déclaration de pointeurs vers les objets LVGL utilisés dans l'interface
static lv_obj_t * slider_label;  // Label qui affichera la valeur du slider
static lv_obj_t * bleu;          // Label pour la composante bleue
static lv_obj_t * vert;          // Label pour la composante verte
static lv_obj_t * rouge;         // Label pour la composante rouge
lv_obj_t * AUTO = NULL;          // Bouton AUTO (mode automatique)
bool lance = false;              // Variable de contrôle pour activer ou non le mode automatique

// (Répétition inutile, déjà défini au-dessus)
typedef FILE * my_file_t; // Ou SdFile * pour Arduino SD

// --- Fonctions de rappel pour le système de fichiers LVGL ---

// Fonction appelée lorsqu’un fichier est ouvert par LVGL
static void * my_open_cb(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode)
{
    // Conversion du mode LVGL en mode de fichier standard C
    const char * fs_mode = "";
    if (mode == LV_FS_MODE_RD) fs_mode = "rb"; // Lecture binaire
    else if (mode == LV_FS_MODE_WR) fs_mode = "wb"; // Écriture binaire
    else if (mode == (LV_FS_MODE_RD | LV_FS_MODE_WR)) fs_mode = "rb+"; // Lecture/écriture binaire
    else return NULL; // Mode non supporté

    // Ouverture réelle du fichier
    my_file_t f = fopen(path, fs_mode);
    return (void *)f;
}

// Fonction appelée pour fermer un fichier
static lv_fs_res_t my_close_cb(lv_fs_drv_t * drv, void * file_p)
{
    fclose((my_file_t)file_p); // Fermeture standard du fichier
    return LV_FS_RES_OK;
}

// Fonction appelée pour lire un fichier
static lv_fs_res_t my_read_cb(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
    *br = fread(buf, 1, btr, (my_file_t)file_p); // Lecture d’un bloc de données
    return LV_FS_RES_OK;
}

// Fonction appelée pour se déplacer dans un fichier (positionner le curseur)
static lv_fs_res_t my_seek_cb(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence)
{
    fseek((my_file_t)file_p, pos, (int)whence); // Seek standard
    return LV_FS_RES_OK;
}

// Fonction appelée pour récupérer la position actuelle dans un fichier
static lv_fs_res_t my_tell_cb(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
    *pos_p = ftell((my_file_t)file_p); // Tell standard
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

// Définition des broches utilisées pour contrôler la LED RGB via PWM
#define R  10  // Broche pour la couleur rouge
#define G  9   // Broche pour la couleur verte
#define B  11  // Broche pour la couleur bleue

// Inclusion du pilote spécifique pour LVGL adapté à ta plateforme (Arduino ou autre)
#include "lvglDrivers.h"

// #include "demos/lv_demos.h" // (optionnel) Démo LVGL, à décommenter pour tester les composants LVGL

// Déclaration du slider global (créé plus tard dans l'interface)
lv_obj_t * slider = NULL;

// Coefficient de pente utilisé pour les calculs des couleurs (valeur fixée à -1 ici)
int A = -255 / 255;  // = -1



// Fonction pour calculer la valeur du canal rouge en fonction du slider (sli)
static int redled(int sli)
{
  int red;     // valeur de sortie
  int slideb;  // base de décalage sur l’axe du slider
  int Y;       // valeur de référence pour le calcul

  // 1re phase : Vert augmente, Rouge reste à 255
  if ((sli >= 0) && (sli < 1*255)) {
    red = 255;
  }

  // 2e phase : Rouge descend, Vert reste
  else if ((sli >= 1*255) && (sli < 2*255)) {
    slideb = 1*255;
    Y = 255;
    red = A * (sli - slideb) + Y;
  }

  // 5e phase : Rouge remonte, Bleu reste
  else if ((sli >= 4*255) && (sli < 5*255)) {
    slideb = 4*255;
    Y = 0;
    red = -A * (sli - slideb) + Y;
  }

  // 6e phase : Bleu descend, Rouge reste
  else if ((sli >= 5*255) && (sli < 6*255)) {
    red = 255;
  }

  // Valeur par défaut (en dehors des plages définies)
  else {
    red = 0;
  }

  return red;
}



// Fonction pour calculer la valeur du canal vert
static int greenled(int sli)
{
  int gre;
  int slideb;
  int Y;

  // 1re phase : Vert monte
  if ((sli >= 0) && (sli < 1*255)) {
    slideb = 0;
    Y = 0;
    gre = -A * (sli - slideb) + Y;
  }

  // 2e phase : Rouge descend, Vert reste
  else if ((sli >= 1*255) && (sli < 2*255)) {
    gre = 255;
  }

  // 3e phase : Bleu monte, Vert reste
  else if ((sli >= 2*255) && (sli < 3*255)) {
    gre = 255;
  }

  // 4e phase : Vert descend
  else if ((sli >= 3*255) && (sli < 4*255)) {
    slideb = 3*255;
    Y = 255;
    gre = A * (sli - slideb) + Y;
  }

  else {
    gre = 0;
  }

  return gre;
}


// Fonction pour calculer la valeur du canal bleu
static int blueled(int sli)
{
  int blue;
  int slideb;
  int Y;

  // 3e phase : Bleu monte
  if ((sli >= 2*255) && (sli < 3*255)) {
    slideb = 2*255;
    Y = 0;
    blue = -A * (sli - slideb) + Y;
  }

  // 4e phase : Vert descend, Bleu reste
  else if ((sli >= 3*255) && (sli < 4*255)) {
    blue = 255;
  }

  // 5e phase : Rouge monte, Bleu reste
  else if ((sli >= 4*255) && (sli < 5*255)) {
    blue = 255;
  }

  // 6e phase : Bleu descend
  else if ((sli >= 5*255) && (sli < 6*255)) {
    slideb = 5*255;
    Y = 255;
    blue = A * (sli - slideb) + Y;
  }

  else {
    blue = 0;
  }

  return blue;
}


// Compteur utilisé pour faire évoluer automatiquement la couleur dans le temps
static int animation_step = 0;

// Variable de bascule utilisée pour activer ou désactiver le mode automatique
static int plus = 0;






// Fonction appelée lorsqu'on appuie sur le bouton AUTO
// Elle permet de lancer ou d'arrêter le mode de changement automatique des LED
static void ledauto(lv_event_t * e) {
    // Récupère le type d’événement (ici, un clic)
    lv_event_code_t bp = lv_event_get_code(e);

    // Récupère l’objet qui a déclenché l’événement (ici le bouton AUTO)
    lv_obj_t * btn = (lv_obj_t *) lv_event_get_target(e);
 
    // Si le bouton AUTO est cliqué
    if (bp == LV_EVENT_CLICKED) {

        // Active le mode automatique
        lance = true;
        plus++;  // Incrémente le compteur d'appui

        // Si on appuie une deuxième fois
        if (plus == 2) {
            // Réactive le slider (pour le contrôle manuel)
            lv_obj_set_state(slider, LV_STATE_DISABLED, false);

            // Désactive l'automatisation
            lance = false;
            plus = 0;
        }
    }
}





// Fonction appelée lorsqu’on déplace le slider
// Elle met à jour les valeurs RGB de la LED et les affiche à l’écran
static void slider_event_cb(lv_event_t * e)
{
    // Récupère le slider qui a généré l’événement
    lv_obj_t * slider = lv_event_get_target_obj(e);

    // Buffers pour les textes affichés
    char buf[8];  // Pour le pourcentage du slider
    char r[8];    // Pour la valeur rouge
    char g[8];    // Pour la valeur verte
    char b[8];    // Pour la valeur bleue

    // Affiche la position actuelle du slider (0 à 1530)
    lv_snprintf(buf, sizeof(buf), "%d%%", ((int)lv_slider_get_value(slider)));
    lv_label_set_text(slider_label, buf);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // Récupère les valeurs RGB correspondantes
    int red   = redled(lv_slider_get_value(slider));
    int blue  = blueled(lv_slider_get_value(slider));
    int green = greenled(lv_slider_get_value(slider));

    // Met à jour les labels avec les valeurs RGB
    lv_snprintf(r, sizeof(r), "%d%%", red);
    lv_snprintf(g, sizeof(g), "%d%%", green);
    lv_snprintf(b, sizeof(b), "%d%%", blue);

    lv_label_set_text(rouge, r);
    lv_label_set_text(vert, g);
    lv_label_set_text(bleu, b);

    // Positionnement des labels sous le slider
    lv_obj_align_to(rouge, slider, LV_ALIGN_OUT_BOTTOM_MID, -40, 30);
    lv_obj_align_to(vert, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
    lv_obj_align_to(bleu, slider, LV_ALIGN_OUT_BOTTOM_MID, 40, 30);

    // Applique les couleurs via PWM sur les broches R, G, B
    analogWrite(R, red);    
    analogWrite(G, green);    
    analogWrite(B, blue); 
}







// Objets globaux pour interface : image de fond, bouton OFF, deuxième image de fond
lv_obj_t * img_bg = NULL;
lv_obj_t * btn2 = NULL;
lv_obj_t * img_bg3 = NULL;

// Déclaration de styles pour LVGL (non utilisés ici sauf styles slider plus bas)
static lv_style_t style_main;
static lv_style_t style_indicator;
static lv_style_t style_knob;
static lv_style_t style_pressed_color;


// Fonction exécutée lorsqu’on clique sur le bouton "ON"
static void ledon (lv_event_t * e)
{
    // Récupère le type d’événement déclenché
    lv_event_code_t bp = lv_event_get_code(e);

    // Récupère le bouton qui a déclenché l’événement
    lv_obj_t * btn = (lv_obj_t *) lv_event_get_target(e);
 
    // Si l'événement est un clic
    if (bp == LV_EVENT_CLICKED){

        // Déclaration d’un label local
        lv_obj_t * label;

        // Déclaration de l’image utilisée comme fond (colorbar)
        LV_IMG_DECLARE(colorbar);

        // ➤ Création du bouton AUTO (pour activer l’animation automatique)
        AUTO = lv_button_create(lv_screen_active());                // Crée un nouveau bouton
        lv_obj_align(AUTO, LV_ALIGN_CENTER, 0, -50);                // Positionne le bouton au-dessus du centre
        Serial.printf("align ok");
        lv_obj_remove_flag(AUTO, LV_OBJ_FLAG_PRESS_LOCK);          // Empêche le bouton de rester "enfoncé"

        label = lv_label_create(AUTO);                              // Crée un texte à l'intérieur du bouton
        lv_label_set_text(label, "Auto");                           // Définit le texte
        Serial.printf("text ok");
        lv_obj_center(label);                                       // Centre le texte dans le bouton
        lv_obj_add_event_cb(AUTO, ledauto, LV_EVENT_CLICKED, NULL);// Associe l’événement click à la fonction ledauto()

        // ➤ Création de l’image de fond (colorbar derrière le slider)
        img_bg = lv_img_create(lv_screen_active());                 // Crée une image
        lv_img_set_src(img_bg, &colorbar);                          // Associe l’image déclarée
        lv_obj_align(img_bg, LV_ALIGN_CENTER, 0, 0);                // Centre l’image

        // ➤ Configuration du style de l’indicateur du slider (barre colorée)
        static lv_style_t style_indicator;
        lv_style_init(&style_indicator);
        lv_style_set_bg_color(&style_indicator, lv_palette_main(LV_PALETTE_CYAN));
        lv_style_set_radius(&style_indicator, LV_RADIUS_CIRCLE);

        // ➤ Configuration du style du "knob" (bouton circulaire du slider)
        static lv_style_t style_knob;
        lv_style_init(&style_knob);
        lv_style_set_bg_color(&style_knob, lv_palette_main(LV_PALETTE_CYAN));
        lv_style_set_radius(&style_knob, LV_RADIUS_CIRCLE);
        lv_style_set_border_width(&style_knob, 2);
        lv_style_set_border_color(&style_knob, lv_palette_darken(LV_PALETTE_CYAN, 2));
        lv_style_set_pad_all(&style_knob, 8);  // Rendre le bouton plus grand

        // ➤ Création du slider (curseur horizontal)
        slider = lv_slider_create(lv_screen_active());
        lv_obj_remove_style_all(slider);                              // Supprime les styles par défaut
        lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL); // Lie l’événement de déplacement

        lv_obj_set_size(slider, 420, 30);                             // Dimensions du slider

        lv_obj_set_style_width(slider, 30, LV_PART_KNOB);            // Largeur du bouton
        lv_obj_set_style_height(slider, 5, LV_PART_KNOB);            // Hauteur du bouton
        lv_obj_set_style_radius(slider, 0, LV_PART_KNOB);            // Pas de coins arrondis

        // ➤ Appliquer les styles personnalisés
        lv_obj_add_style(slider, &style_indicator, LV_PART_INDICATOR);
        lv_obj_add_style(slider, &style_knob, LV_PART_KNOB);

        // ➤ Définir la plage du slider (valeurs entre 0 et 1530 pour animation complète RGB)
        lv_slider_set_range(slider, 0, 1530);

        // ➤ Création des labels pour afficher les valeurs
        slider_label = lv_label_create(lv_screen_active());
        lv_label_set_text(slider_label, "0%");

        rouge = lv_label_create(lv_screen_active());
        vert = lv_label_create(lv_screen_active());
        bleu = lv_label_create(lv_screen_active());
        lv_label_set_text(rouge, "0%");
        lv_label_set_text(vert, "0%");
        lv_label_set_text(bleu, "0%");

        // ➤ Changer la couleur des textes (blanc)
        lv_obj_set_style_text_color(slider_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_color(rouge, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_color(vert, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_color(bleu, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

        // ➤ Positionnement du slider et des textes
        lv_obj_align(slider, LV_ALIGN_CENTER, 0, 0); // Centrer le slider
        lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
        lv_obj_align_to(rouge, slider, LV_ALIGN_OUT_BOTTOM_MID, -40, 30);
        lv_obj_align_to(vert, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
        lv_obj_align_to(bleu, slider, LV_ALIGN_OUT_BOTTOM_MID, 40, 30);

        // Positionner correctement l’ensemble dans l’écran
        lv_obj_center(slider);
    }
}


// Fonction appelée quand l'utilisateur clique sur le bouton OFF
static void ledoff(lv_event_t * e)
{
    // Récupère le type d’événement (clic, changement d'état, etc.)
    lv_event_code_t bp = lv_event_get_code(e);

    // Récupère l'objet qui a déclenché l'événement (ici, le bouton OFF)
    lv_obj_t * btn = (lv_obj_t *) lv_event_get_target(e);

    // Si l'événement est un clic sur le bouton
    if (bp == LV_EVENT_CLICKED){

        // Désactive le mode automatique
        lance = false;

        // Réinitialise le compteur d'animation
        animation_step = 0;

        // Éteint les 3 canaux RGB en mettant les PWM à 0
        analogWrite(R, 0);    
        analogWrite(G, 0);    
        analogWrite(B, 0);

        // Supprime tous les éléments visuels créés par la fonction ledon()

        lv_obj_del(img_bg);         // Supprime l'image de fond (barre de couleurs)
        img_bg = NULL;

        lv_obj_del(slider_label);   // Supprime le label du slider
        slider_label = NULL;

        lv_obj_del(slider);         // Supprime le slider lui-même
        slider = NULL;

        lv_obj_del(rouge);          // Supprime les labels des couleurs
        rouge = NULL;

        lv_obj_del(vert);
        vert = NULL;

        lv_obj_del(bleu);
        bleu = NULL;

        lv_obj_del(AUTO);           // Supprime le bouton AUTO
        AUTO = NULL;
    }
}




// Fonction appelée au démarrage du programme pour configurer l’interface
void mySetup()
{
    // Configure les broches comme sorties pour les canaux RGB
    pinMode(R, OUTPUT);
    pinMode(G, OUTPUT);
    pinMode(B, OUTPUT);

    // -- Déclaration et affichage de l’image d’accueil --
    lv_obj_t * label;

    LV_IMG_DECLARE(mhimg); // Déclare une image externe nommée mhimg

    img_bg3 = lv_img_create(lv_screen_active());  // Crée un objet image
    lv_img_set_src(img_bg3, &mhimg);              // Associe l'image
    lv_obj_align(img_bg3, LV_ALIGN_CENTER, 0, 0);  // Centre l’image
    lv_image_set_scale(img_bg3, 480);             // (fonction probablement erronée ici, à vérifier)

    // -- Bouton ON --

    lv_obj_t * btn1 = lv_button_create(lv_screen_active());         // Crée un bouton
    lv_obj_align(btn1, LV_ALIGN_CENTER, 210, -100);                 // Le place à droite en haut
    Serial.printf("align ok");
    lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);              // Empêche le bouton de rester enfoncé

    label = lv_label_create(btn1);                                  // Crée un label pour le bouton
    lv_label_set_text(label, "On");                                 // Texte du bouton
    Serial.printf("text ok");
    lv_obj_center(label);                                           // Centre le texte dans le bouton
    lv_obj_add_event_cb(btn1, ledon, LV_EVENT_CLICKED, NULL);       // Associe l’action à la fonction ledon()

    // -- Bouton OFF --

    btn2 = lv_button_create(lv_screen_active());                    // Crée le bouton OFF
    lv_obj_align(btn2, LV_ALIGN_CENTER, 210, 100);                  // Le place à droite en bas
    Serial.printf("align ok");
    lv_obj_remove_flag(btn2, LV_OBJ_FLAG_PRESS_LOCK);

    label = lv_label_create(btn2);                                  // Ajoute un texte au bouton OFF
    lv_label_set_text(label, "Off");
    Serial.printf("text ok");
    lv_obj_center(label);
    lv_obj_add_event_cb(btn2, ledoff, LV_EVENT_CLICKED, NULL);      // Associe le bouton à la fonction ledoff()

    // -- Titre de l’interface --

    lv_obj_t * titre = lv_label_create(lv_screen_active());         // Crée un titre
    lv_label_set_text(titre, "CONTROLEUR LED");                     // Texte du titre

    lv_obj_set_style_text_color(titre, lv_color_hex(0xFFFFFF), LV_PART_MAIN); // Met le texte en blanc
    lv_obj_align(titre, LV_ALIGN_CENTER, 0, -100);                  // Positionne le titre en haut de l’écran
}


// Boucle principale du programme (exécutée en continu par la carte)
void loop()
{
    // Si le mode automatique est activé (après appui sur le bouton AUTO)
    if (lance == true) {

        // Désactive le slider pour que l’utilisateur ne puisse pas l’utiliser
        lv_obj_set_state(slider, LV_STATE_DISABLED, true);

        // Incrémente la variable de l'animation à chaque boucle
        animation_step++;

        // Si on atteint la fin du cycle (valeur max), on recommence à 0
        if (animation_step >= 1530) {
            animation_step = 0;
        }

        // Stocke la valeur courante de l’animation (entre 0 et 1530)
        int autosli = animation_step;

        // Buffers pour l'affichage des valeurs (slider + RGB)
        char buf[8];
        char r[8];
        char g[8];
        char b[8];

        // Affiche la position actuelle de l'animation dans le label du slider
        lv_snprintf(buf, sizeof(buf), "%d%%", autosli);
        lv_label_set_text(slider_label, buf);
        lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

        // Calcule les valeurs RGB en fonction de la position du slider automatique
        int red   = redled(autosli);
        int blue  = blueled(autosli);
        int green = greenled(autosli);

        // Affiche les valeurs RGB à l'écran
        lv_snprintf(r, sizeof(r), "%d%%", red);
        lv_snprintf(g, sizeof(g), "%d%%", green);
        lv_snprintf(b, sizeof(b), "%d%%", blue);

        lv_label_set_text(rouge, r);
        lv_label_set_text(vert, g);
        lv_label_set_text(bleu, b);

        // Positionne les labels sous le slider
        lv_obj_align_to(rouge, slider, LV_ALIGN_OUT_BOTTOM_MID, -40, 30);
        lv_obj_align_to(vert, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
        lv_obj_align_to(bleu, slider, LV_ALIGN_OUT_BOTTOM_MID, 40, 30);

        // Envoie les signaux PWM aux broches R, G, B pour afficher la couleur
        analogWrite(R, red);    
        analogWrite(G, green);    
        analogWrite(B, blue); 

        // Attente courte pour ralentir la vitesse d'animation
        delay(20);
    }

    // Fonction obligatoire de LVGL à appeler régulièrement pour actualiser l’affichage
    lv_timer_handler();

    // Remarque :
    // Si tu veux que la couleur change en fonction d’un autre paramètre (ex. luminosité de l’écran),
    // tu peux ici récupérer la valeur du capteur ou d’un autre composant.
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

#include "initialisation.h"

#include "knob.h"

#define LOG_SQLITE3_ERROR(db) knob_log(KNOB_ERROR, "%s:%d: SQLITE3 ERROR: %s\n", __FILE__, __LINE__, sqlite3_errmsg(db))

//--ADD TABLE
const char* content[] = {
    "CREATE TABLE IF NOT EXISTS lieux(id_lieu INTEGER PRIMARY KEY NOT NULL,nom TEXT NOT NULL UNIQUE,description TEXT NOT NULL);",
    "CREATE TABLE IF NOT EXISTS objets(id_objet INTEGER PRIMARY KEY NOT NULL,nom TEXT NOT NULL,description TEXT NOT NULL,id_lieu INT,FOREIGN KEY (id_lieu) REFERENCES lieux(id_lieu));",
    "CREATE TABLE IF NOT EXISTS joueurs(id_joueur INTEGER PRIMARY KEY NOT NULL,nom TEXT NOT NULL, vie INT NOT NULL, force INT NOT NULL, position_id INT, FOREIGN KEY (position_id) REFERENCES lieux(id_lieu));",
    "CREATE TABLE IF NOT EXISTS pnj(id_pnj INTEGER PRIMARY KEY NOT NULL,nom TEXT NOT NULL UNIQUE,description TEXT NOT NULL,dialogue TEXT NOT NULL,dialogue_quete_accepte TEXT NOT NULL,id_lieu INT,FOREIGN KEY (id_lieu) REFERENCES lieux(id_lieu));",
    "CREATE TABLE IF NOT EXISTS ennemis(id_ennemi INTEGER PRIMARY KEY NOT NULL,nom TEXT NOT NULL,vie INT NOT NULL, force INT NOT NULL,quete_principal_ou_secondaire INT,id_lieu INT,FOREIGN KEY (id_lieu) REFERENCES lieux(id_lieu));",
    "CREATE TABLE IF NOT EXISTS inventaire(id_inventaire INTEGER PRIMARY KEY NOT NULL,id_joueur INT,id_objet INT,FOREIGN KEY (id_joueur) REFERENCES joueurs(id_joueur),FOREIGN KEY (id_objet) REFERENCES objets(id_objet));",
    "CREATE TABLE IF NOT EXISTS quetes(id_quete INTEGER PRIMARY KEY NOT NULL,description TEXT NOT NULL,est_quete_principale INT,est_complete INT DEFAULT 0,id_joueur INT,id_pnj INT,FOREIGN KEY (id_joueur) REFERENCES joueurs(id_joueur),FOREIGN KEY (id_pnj) REFERENCES pnj(id_pnj));",
    "CREATE TABLE IF NOT EXISTS pnj_quetes(id_pnj_quete INTEGER PRIMARY KEY NOT NULL,id_pnj INT,id_quete INT,FOREIGN KEY (id_pnj) REFERENCES pnj(id_pnj),FOREIGN KEY (id_quete) REFERENCES quetes(id_quete));"
};
void create_state_table(sqlite3 *db)
{ // fonction pour lore dans la base de données
    sqlite3_stmt *stmt = NULL;

    for (int i = 0; i < 8; i++)
    {
        int ret = sqlite3_prepare_v2(db, content[i], -1, &stmt, NULL);

        if (ret != SQLITE_OK)
        {
            LOG_SQLITE3_ERROR(db);
        }
    
        ret = sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);
}

//ADD LIEUX
const char *lieux[] = {
    "Mine Hantée",
    "Village Abandonné",
    "Cimetière",
    "Maison du Joueur"
};

const char *description_lieu[] = {
    "Une mine de 12 kilomètre de long creusée par le peuple des nains. Un jour une pluie forte à créer un grand déluge inondant la mine et tuant les nains à l'intérieur. Une rumeur disant que c'est le roi qui demandé à un sorcier Troll pour provoquer cette pluie pour prendre le contrôle de la mine. Des personnes ayant fait un tour dans la mine ont raconté qu'il y entendait des voix et voyait des silhouettes brume ayant la taille d'un nain.",
    "Lors d'une nuit sombre un étrange dragon à déserter son nid et a voulu s'en prendre à l'humain. Il a décidé de s'installer dans un village. Il brûla en entier le village. Impuissant les humains ont dû abandonner leur village. Une autre rumeur propagée par le peuple des Elfes dit que le dragon a été ensorcelé par une magie noire qui tire son origine du pays des Trolls.",
    "Un cimetière tout à fait normal le jour, mais chaque nuit un brouillard épais apparaît et plusieurs témoignages ont témoigné voir une silhouette sans tête se promener sur le dos d'un cheval de guerre. Ce qui est plus étrange, c'est que cette silhouette n'a pas de tête.",
    "La maison du joueur"
};

int taille_lieu = sizeof(lieux) / sizeof(lieux[0]);
void add_lieux(sqlite3 *db)
{
    if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
    {
        LOG_SQLITE3_ERROR(db);
        return;
    }
    sqlite3_stmt *stmt = NULL;

    char content[] =
        "INSERT INTO lieux(nom,description)\n"
        "VALUES\n"
        "(?,?);";

    int ret = sqlite3_prepare_v2(db, content, -1, &stmt, NULL);

    for (int i = 0; i < taille_lieu; i++)
    {
        //--
        if (sqlite3_bind_text(stmt, 1, lieux[i], -1, SQLITE_STATIC) != SQLITE_OK)
        {
            LOG_SQLITE3_ERROR(db);
        }
        if (sqlite3_bind_text(stmt, 2, description_lieu[i], -1, SQLITE_STATIC) != SQLITE_OK)
        {
            LOG_SQLITE3_ERROR(db);
        }

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            LOG_SQLITE3_ERROR(db);
        }
        //--
        //ret = sqlite3_step(stmt);
        
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }

    ret = sqlite3_step(stmt);
    
    sqlite3_finalize(stmt);

    if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
    {
        //LOG_SQLITE3_ERROR(db);
    }
}


//ADD ENNEMI
const char *ennemis[] = {
    "Zombie",
    "Fantôme nain",
    "Dragon",
    "Dullahans"
};

void add_ennemi(sqlite3 *db)
{
    if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
    {
        LOG_SQLITE3_ERROR(db);
        return;
    }
    sqlite3_stmt *stmt = NULL;

    char content[] =
        "INSERT INTO ennemis(nom,vie,force,quete_principal_ou_secondaire,id_lieu)\n"
        "VALUES\n"
        "(?,?,?,?,?);";

    int ret = sqlite3_prepare_v2(db, content, -1, &stmt, NULL);

    int vie;
    int force;
    int quete_principal_ou_secondaire;

    for (int i = 1; i < 4; i++)
    {
        //Mine hantée
        if (i == 1)
        {
            for (int j = 0; j < 2; j++)
            {
                if (j == 0)
                {
                    vie = 50;
                    force = 90;
                    quete_principal_ou_secondaire = 0;
                } else {
                    vie = 100;
                    force = 100;
                    quete_principal_ou_secondaire = 1;
                }
                
                if (sqlite3_bind_text(stmt, 1, ennemis[j], -1, SQLITE_STATIC) != SQLITE_OK)
                {
                    LOG_SQLITE3_ERROR(db);
                }
                if(sqlite3_bind_int(stmt,2,vie) != SQLITE_OK){
                    LOG_SQLITE3_ERROR(db);
                }
                if(sqlite3_bind_int(stmt,3,force) != SQLITE_OK){
                    LOG_SQLITE3_ERROR(db);
                }
                if(sqlite3_bind_int(stmt,4,quete_principal_ou_secondaire) != SQLITE_OK){
                    LOG_SQLITE3_ERROR(db);
                }
                if(sqlite3_bind_int(stmt,5,i) != SQLITE_OK){
                    LOG_SQLITE3_ERROR(db);
                }

                if (sqlite3_step(stmt) != SQLITE_DONE)
                {
                    LOG_SQLITE3_ERROR(db);
                }
                //--
                //ret = sqlite3_step(stmt);
                
                sqlite3_reset(stmt);
                sqlite3_clear_bindings(stmt);
            }
            
        }
        //Village abandonné
        else if(i == 2){
            for (int j = 0; j < 3; j++)
            {
                if (j == 0 || j == 2)
                {
                    if (j == 0)
                    {
                        vie = 50;
                        force = 95;
                        quete_principal_ou_secondaire = 0;
                    } else if (j == 2) {
                        vie = 130;
                        force = 130;
                        quete_principal_ou_secondaire = 1;
                    }

                    if (sqlite3_bind_text(stmt, 1, ennemis[j], -1, SQLITE_STATIC) != SQLITE_OK)
                    {
                        LOG_SQLITE3_ERROR(db);
                    }
                    if(sqlite3_bind_int(stmt,2,vie) != SQLITE_OK){
                        LOG_SQLITE3_ERROR(db);
                    }
                    if(sqlite3_bind_int(stmt,3,force) != SQLITE_OK){
                        LOG_SQLITE3_ERROR(db);
                    }
                    if(sqlite3_bind_int(stmt,4,quete_principal_ou_secondaire) != SQLITE_OK){
                        LOG_SQLITE3_ERROR(db);
                    }
                    if(sqlite3_bind_int(stmt,5,i) != SQLITE_OK){
                        LOG_SQLITE3_ERROR(db);
                    }

                    if (sqlite3_step(stmt) != SQLITE_DONE)
                    {
                        LOG_SQLITE3_ERROR(db);
                    }
                    //--
                    //ret = sqlite3_step(stmt);
                    
                    sqlite3_reset(stmt);
                    sqlite3_clear_bindings(stmt);
                }
                
            }
            
        }
        //Cimétière
        else if(i == 3){
            for (int j = 0; j < 4; j++)
            {
                if(j == 0 || j == 3){
                    if (j == 0)
                    {
                        vie = 50;
                        force = 100;
                        quete_principal_ou_secondaire = 0;
                    } else if(j == 3) {
                        vie = 200;
                        force = 200;
                        quete_principal_ou_secondaire = 1;
                    }

                    if (sqlite3_bind_text(stmt, 1, ennemis[j], -1, SQLITE_STATIC) != SQLITE_OK)
                    {
                        LOG_SQLITE3_ERROR(db);
                    }
                    if(sqlite3_bind_int(stmt,2,vie) != SQLITE_OK){
                        LOG_SQLITE3_ERROR(db);
                    }
                    if(sqlite3_bind_int(stmt,3,force) != SQLITE_OK){
                        LOG_SQLITE3_ERROR(db);
                    }
                    if(sqlite3_bind_int(stmt,4,quete_principal_ou_secondaire) != SQLITE_OK){
                        LOG_SQLITE3_ERROR(db);
                    }
                    if(sqlite3_bind_int(stmt,5,i) != SQLITE_OK){
                        LOG_SQLITE3_ERROR(db);
                    }

                    if (sqlite3_step(stmt) != SQLITE_DONE)
                    {
                        LOG_SQLITE3_ERROR(db);
                    }
                    //--
                    //ret = sqlite3_step(stmt);
                    
                    sqlite3_reset(stmt);
                    sqlite3_clear_bindings(stmt);
                }
            }
        }
        
    }

    ret = sqlite3_step(stmt);
    
    sqlite3_finalize(stmt);

    if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
    {
        //LOG_SQLITE3_ERROR(db);
    }
}

//-------------------ADD OBJET
const char *objets[] = {
    "Hâche",
    "Torche",
    "Couteau"
};

const char *description_objets[] = {
    "Une arme lente, mais causant de grand dégât. Durabilité forte.",
    "Une torche qui produit d ela lumière pour éclairer les endroits sombre. Elle comprend un enchantement magique qui repousse un certain type d'esprit maléfique.",
    "Une arme rapide, causant des dégât minime mais avec des effets de saignement. Durabilité faible.",
};

int taille_objet = sizeof(objets) / sizeof(objets[0]);

void add_objets(sqlite3 *db)
{
    if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
    {
        LOG_SQLITE3_ERROR(db);
        return;
    }
    sqlite3_stmt *stmt = NULL;

    char content[] =
        "INSERT INTO objets(nom,description,id_lieu)\n"
        "VALUES\n"
        "(?,?,?);";

    int ret = sqlite3_prepare_v2(db, content, -1, &stmt, NULL);
    
    for (int i = 0; i < taille_lieu - 1; i++)
    {
        int objet = rand() %3;
        //--
        if (sqlite3_bind_text(stmt, 1, objets[objet], -1, SQLITE_STATIC) != SQLITE_OK)
        {
            LOG_SQLITE3_ERROR(db);
        }
        if (sqlite3_bind_text(stmt, 2, description_objets[objet], -1, SQLITE_STATIC) != SQLITE_OK)
        {
            LOG_SQLITE3_ERROR(db);
        }
        if (sqlite3_bind_int(stmt, 3, i + 1) != SQLITE_OK)
        {
            LOG_SQLITE3_ERROR(db);
        }
        
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            LOG_SQLITE3_ERROR(db);
        }
        //--
        // ret = sqlite3_step(stmt);
        
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);

    }

    ret = sqlite3_step(stmt);
    
    sqlite3_finalize(stmt);

    if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
    {
        LOG_SQLITE3_ERROR(db);
    }
}

//----ADD JOUEUR
void add_inscription_joueur(sqlite3* db, char* nom_joueur){

    if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK) {
        LOG_SQLITE3_ERROR(db);
        return;
    }
    sqlite3_stmt* stmt = NULL;
    char content[] =
    "INSERT INTO joueurs(nom,vie,force,position_id)\n"
    "VALUES\n"
    "(?,?,?,?);";
    int ret = sqlite3_prepare_v2(db,content,-1,&stmt,NULL);
    int vie = 100;
    int force = 100;
    int position_id = 5; //le joueur est d'abord chez lui

    if(sqlite3_bind_text(stmt,1,nom_joueur,-1,SQLITE_STATIC) != SQLITE_OK){
        LOG_SQLITE3_ERROR(db);
    }
    if(sqlite3_bind_int(stmt,2,vie) != SQLITE_OK){
        LOG_SQLITE3_ERROR(db);
    }
    if(sqlite3_bind_int(stmt,3,force) != SQLITE_OK){
        LOG_SQLITE3_ERROR(db);
    }
    if(sqlite3_bind_int(stmt,4,position_id) != SQLITE_OK){
        LOG_SQLITE3_ERROR(db);
    }
    ret = sqlite3_step(stmt);
    if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK) {
        LOG_SQLITE3_ERROR(db);
    }
}


//----- ADD PNJ
const char *pnj[] = {
    "Nain",
    "Elfe",
    "Esclave humaine",
    "Passante humaine"
};
const char *description_pnj[] = {
    "Nain survivant au déluge de la mine.",
    "Membre du village des elfes.",
    "Un Homme kidnappé qui était un sujet d'expérience de Drak Green",
    "Femme affolée car elle cherche sa fille depuis le matin."
};
const char *dialogue[] = {
    "Bienvenue jeune héros, je parie que toi aussi tu cherches le parchemin de foudre. J'étais avant un ouvrier de cette mine et je la connais bien. Si tu arrives a me raporter une pierre de lune je te montrerais un raccourcis. ",
    "Bienvenu jeune humain, toi aussi tu veux chasser ce dragon. Je te conseil de me suivre pour passer l'epreuve du feu. Ça t'aidera grandement.",
    "J'ai été un esclave de Dark Green et je veux sa mort",
    "S'il vous plaît aidé moi, je cherche ma fille depuis le matin je ne l'a voit plus si vous m'aider je vous donnerai un cadeau qui vous aidera grandement, j'ai vu que vous vous rendrez au cimetière."
};

const char *dialogue_quete_accepte[] = {
    "Jeune héros la pierre de lune se trouve encore plus facilement lors d'une clair de lune",
    "Pour passer l'épreuve de feu tu dois avoir un mentale d'acier",
    "Soit courageux jeune homme",
    "Au dernière nouvelle elle se trouverait au marché"
};
const char *lieu_pnj[] = {
    "1",
    "2",
    "3"
};
void add_pnj(sqlite3 *db)
{
    if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
    {
        LOG_SQLITE3_ERROR(db);
        return;
    }
    sqlite3_stmt *stmt = NULL;

    char content[] =
        "INSERT INTO pnj(nom,description,dialogue,dialogue_quete_accepte,id_lieu)\n"
        "VALUES\n"
        "(?,?,?,?,?);";

    int ret = sqlite3_prepare_v2(db, content, -1, &stmt, NULL);

    for (int i = 0; i < 3; i++)
    {
        //--
        if (sqlite3_bind_text(stmt, 1, pnj[i], -1, SQLITE_STATIC) != SQLITE_OK)
        {
            LOG_SQLITE3_ERROR(db);
        }
        if (sqlite3_bind_text(stmt, 2, description_pnj[i], -1, SQLITE_STATIC) != SQLITE_OK)
        {
            LOG_SQLITE3_ERROR(db);
        }
        if (sqlite3_bind_text(stmt, 3, dialogue[i], -1, SQLITE_STATIC) != SQLITE_OK)
        {
            LOG_SQLITE3_ERROR(db);
        }
        if (sqlite3_bind_text(stmt, 4, dialogue_quete_accepte[i], -1, SQLITE_STATIC) != SQLITE_OK)
        {
            LOG_SQLITE3_ERROR(db);
        }
        if (sqlite3_bind_text(stmt, 5, lieu_pnj[i], -1, SQLITE_STATIC) != SQLITE_OK)
        {
            LOG_SQLITE3_ERROR(db);
        }

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            LOG_SQLITE3_ERROR(db);
        }
        //--
        //ret = sqlite3_step(stmt);
        
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }

    ret = sqlite3_step(stmt);
    
    sqlite3_finalize(stmt);

    if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
    {
        LOG_SQLITE3_ERROR(db);
    }
}

//---- ADD QUETES
const char *pnj_quetes[] = {
    "1",
    "1",
    "2",
    "2",
    "3",
    "3"
};
const char *est_quete_principale[] = {
    "1",
    "0",
    "1", 
    "0", 
    "1",
    "0"
};
const char *quetes[] = {
    "Le but de cette quête est de récupérer un ancien parchemin laissé par un grand mage nain. On dit que ce parchemin permet d'invoquer une attaque de foudre qui est utilisable une fois par jour. Requis : Pioche.",
    "Rapporter une pierre de lune pour avoir un raccourci au sein de la mine qui mène directement à la salle du parchemin. Récompense pioche.",
    "Tuer ou chasser le dragon rouge du village. Requis : Magie de résistance au feu.", 
    "Se rendre dans le village des Elfes pour apprendre la magie de résistance au feu.", 
    "Vaincre le cavalier sans tête Dullahans, l'épiste mage Rodrick. Requis : Avoir fait toutes les autres quêtes principales.",
    "Aider une femme humaine à retrouver sa fille. Récompense potion de force." 
};

int taille_quete = sizeof(quetes) / sizeof(quetes[0]);

void add_quetes(sqlite3 *db)
{
    if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
    {
        LOG_SQLITE3_ERROR(db);
        return;
    }
    sqlite3_stmt *stmt = NULL;

    char content[] =
        "INSERT INTO quetes(description,est_complete,est_quete_principale,id_joueur,id_pnj)\n"
        "VALUES\n"
        "(?,?,?,?,?);";

    int ret = sqlite3_prepare_v2(db, content, -1, &stmt, NULL);

    for (int i = 0; i < taille_quete; i++)
    {
        //--
        if (sqlite3_bind_text(stmt, 1, quetes[i], -1, SQLITE_STATIC) != SQLITE_OK)
        {
            LOG_SQLITE3_ERROR(db);
        }
        if(sqlite3_bind_int(stmt,2,0) != SQLITE_OK){
            LOG_SQLITE3_ERROR(db);
        }
        if (sqlite3_bind_text(stmt, 3, est_quete_principale[i], -1, SQLITE_STATIC) != SQLITE_OK)
        {
            LOG_SQLITE3_ERROR(db);
        }
        if(sqlite3_bind_int(stmt,4,1) != SQLITE_OK){
            LOG_SQLITE3_ERROR(db);
        }
        if (sqlite3_bind_text(stmt, 5, pnj_quetes[i], -1, SQLITE_STATIC) != SQLITE_OK)
        {
            LOG_SQLITE3_ERROR(db);
        }

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            LOG_SQLITE3_ERROR(db);
        }
        //--
        //ret = sqlite3_step(stmt);
        
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }

    ret = sqlite3_step(stmt);
    
    sqlite3_finalize(stmt);

    if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
    {
        LOG_SQLITE3_ERROR(db);
    }
}



void initialisation_db(sqlite3* db, const char* nom){
    //CREATION DES TABLES
    create_state_table(db);
    
    //CREATION DES LIEUX
    add_lieux(db);

    //LISTE DES OBJETS
    add_objets(db);

    //CREATION DES ENNEMIS
    add_ennemi(db);

    //CREATION DES PNJ
    add_pnj(db);

    //CREATION QUETES
    add_quetes(db);

    //INSCRIPTION JOUEUR
    add_inscription_joueur(db,nom);
}
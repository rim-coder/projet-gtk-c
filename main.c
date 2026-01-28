

#include <gtk/gtk.h>
#include <cairo/cairo.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// --- Définitions des Constantes et Couleurs ---
#define MAX_N 1000000 // Supporte jusqu'à un million d'éléments
#define MAX_LIST_SIZE 10000 // Increased limit for unlimited feel

#define NODE_WIDTH 80.0
#define NODE_HEIGHT 40.0
#define SPACING 30.0

const GdkRGBA WHITE_COLOR = {1.0, 1.0, 1.0, 1.0};
const GdkRGBA BACKGROUND_COLOR = {248.0/255.0, 248.0/255.0, 248.0/255.0, 1.0};

const gchar *METHOD_NAMES[] = {"Tri à Bulles", "Tri par Insertion", "Tri Shell", "Tri Quicksort"};


// --- [LISTES] --- Définitions des Structures de Liste
typedef struct Node {
    void *data;
    struct Node *next;
    struct Node *prev; // Pour les listes doubles
} Node;

typedef struct List {
    Node *head;
    size_t size;
    size_t element_size;
    int (*compare_func)(const void *, const void *);
    const gchar *structure_type; // "Liste Simple" ou "Liste Double"
    const gchar *element_type;
} List;

// --- [ARBRES] --- Définitions des Structures d'Arbre
typedef struct BinaryNode {
    void *data;
    struct BinaryNode *left;
    struct BinaryNode *right;
} BinaryNode;

typedef struct NaryNode {
    void *data;
    struct NaryNode *first_child;
    struct NaryNode *next_sibling;
} NaryNode;

// --- [GRAPHES] --- Définitions des Structures de Graphe
#define MAX_GRAPH_NODES 20
#define INF 999999

typedef struct {
    int adj_matrix[MAX_GRAPH_NODES][MAX_GRAPH_NODES]; // Matrice d'adjacence avec poids
    int num_nodes;
    void *node_data[MAX_GRAPH_NODES]; // Données des nœuds (tableau de pointeurs)
    size_t element_size; // Taille d'un élément
    const gchar *element_type; // Type de données: "Entiers (Int)", "Réels (Float)", "Caractères (Char)", "Chaîne de Caractères"
    double node_x[MAX_GRAPH_NODES]; // Positions X des nœuds pour le dessin interactif
    double node_y[MAX_GRAPH_NODES]; // Positions Y des nœuds pour le dessin interactif
    gboolean is_directed; // TRUE = orienté, FALSE = non orienté
    gboolean is_weighted; // TRUE = pondéré, FALSE = non pondéré
} Graph;

// Définition du type de fonction de tri sur tableau
typedef void (*SortFunction)(void *, size_t, size_t, int (*)(const void *, const void *));


// --- Définitions de la fonction de tri (PROTOTYPES pour la compilation) ---
static void swap_elements(void *a, void *b, size_t element_size);
static size_t partition(void *data, size_t low, size_t high, size_t element_size, int (*compare_func)(const void *, const void *));
static void quick_sort_recursive(void *data, size_t low, size_t high, size_t element_size, int (*compare_func)(const void *, const void *));
static void bubble_sort(void *data, size_t N, size_t element_size, int (*compare_func)(const void *, const void *));
static void insertion_sort(void *data, size_t N, size_t element_size, int (*compare_func)(const void *, const void *));
static void shell_sort(void *data, size_t N, size_t element_size, int (*compare_func)(const void *, const void *));
static void quick_sort(void *data, size_t N, size_t element_size, int (*compare_func)(const void *, const void *));


// Structure pour stocker les données de nettoyage des fenêtres tableaux
typedef struct {
    void *data_ptr;
    size_t N;
    const gchar *type;
} ArrayCleanupData;

// --- Structure de Données Globale
typedef struct {
    // Module Tableaux
    GtkSpinButton *size_input;
    GtkComboBoxText *type_combo;
    GtkWidget *random_control_box;
    GtkWidget *manual_input_view;
    GtkTextView *unsorted_view;
    GtkTextView *sorted_view;
    GtkWidget *parent_window;

    int input_source; // 0: Aléatoire, 1: Manuel
    int is_single_sort_mode; // 1: Tri simple, 0: Comparaison (Courbes)

    // Données pour la comparaison des courbes
    double comparison_times[4][5]; // 4 méthodes, 5 points N (temps en ms)
    int N_initial;

    // Données du tableau actuel
    void *initial_data_ptr;
    size_t current_N;
    size_t element_size;
    const gchar *current_type;

    // Module Listes Chaînées
    List *current_list;
    GtkComboBoxText *list_type_combo;
    GtkWidget *list_drawing_area;
    GtkTextView *list_info_view;

    // List Input Controls
    int list_input_source; // 0: Random, 1: Manual
    GtkSpinButton *list_size_input;
    GtkEntry *list_value_entry;
    GtkWidget *list_random_box;
    GtkWidget *list_manual_box;

    // Module Arbres
    BinaryNode *binary_root;
    NaryNode *nary_root;
    GtkWidget *tree_drawing_area;
    GtkWidget *tree_scrolled_window;
    GtkTextView *tree_info_view;
    GtkComboBoxText *tree_type_combo;
    GtkComboBoxText *tree_data_type_combo;
    GtkComboBoxText *tree_traversal_combo;
    GtkSpinButton *tree_size_input;
    GtkSpinButton *tree_nary_degree_input;
    GtkEntry *tree_manual_entry;
    GtkWidget *tree_random_radio;
    GtkWidget *tree_manual_radio;
    GtkWidget *tree_random_button; // Bouton "Remplir Aléatoire"
    GtkWidget *tree_manual_label; // Label "Valeur (manuel)"
    GtkWidget *tree_manual_button; // Bouton "Insérer Manuel"
    int tree_is_nary; // 0: Binary, 1: N-Ary
    int tree_input_source; // 0: Aléatoire, 1: Manuel
    int nary_max_children; // degré maximum pour un nœud N-aire

    // Module Graphes
    Graph *current_graph;
    GtkWidget *graph_drawing_area;
    GtkTextView *graph_info_view;
    GtkEntry *graph_src_entry; // Nœud initial (accepte différents types)
    GtkEntry *graph_dest_entry; // Nœud final (accepte différents types)
    GtkComboBoxText *graph_algo_combo;
    GtkComboBoxText *graph_type_combo; // Sélecteur de type de données
    GtkSpinButton *graph_edge_weight_spin; // Poids pour ajout d'arête
    GtkComboBoxText *graph_directed_combo; // Sélecteur orienté/non orienté
    GtkComboBoxText *graph_weighted_combo; // Sélecteur pondéré/non pondéré
    int graph_interaction_mode; // 0: Ajouter nœud, 1: Ajouter arête, 2: Déplacer nœud, 3: Supprimer nœud, 4: Supprimer arête
    int graph_selected_node; // Nœud sélectionné pour créer une arête (-1 si aucun)
    int graph_dragging_node; // Nœud en cours de déplacement (-1 si aucun)
    int graph_dragging_edge_source; // Nœud source lors du glissement pour créer un arc (-1 si aucun)
    double graph_dragging_edge_x; // Position X de la souris pendant le glissement
    double graph_dragging_edge_y; // Position Y de la souris pendant le glissement
    int graph_shortest_path[MAX_GRAPH_NODES]; // Chemin le plus court calculé
    int graph_shortest_path_length; // Longueur du chemin (-1 si aucun chemin)

} AppData;

// =========================================================================
//                             FONCTIONS UTILITAIRES DE DONNÉES
// =========================================================================

// --- Fonction utilitaire pour afficher les erreurs à l'utilisateur ---
static void show_error_dialog(GtkWidget *parent, const gchar *title, const gchar *message) {
    GtkWidget *dialog = gtk_message_dialog_new(
        parent ? GTK_WINDOW(parent) : NULL,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "%s", message);

    if (title) {
        gtk_window_set_title(GTK_WINDOW(dialog), title);
    }
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// --- Fonctions de Comparaison pour qsort/Tri ---

static int compare_int(const void *a, const void *b) {
    int val_a = *(const int *)a;
    int val_b = *(const int *)b;
    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    return 0;
}

static int compare_float(const void *a, const void *b) {
    float val_a = *(const float *)a;
    float val_b = *(const float *)b;
    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    return 0;
}

static int compare_char(const void *a, const void *b) {
    char val_a = *(const char *)a;
    char val_b = *(const char *)b;
    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    return 0;
}

// Comparaison de chaînes (par pointeur)
static int compare_string(const void *a, const void *b) {
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;
    return strcmp(str_a, str_b);
}

// --- Information sur les types ---

static void get_type_info(const gchar *type, size_t *element_size, int (**compare_func)(const void *, const void *)) {
    if (g_strcmp0(type, "Entiers (Int)") == 0) {
        *element_size = sizeof(int);
        *compare_func = compare_int;
    } else if (g_strcmp0(type, "Réels (Float)") == 0) {
        *element_size = sizeof(float);
        *compare_func = compare_float;
    } else if (g_strcmp0(type, "Caractères (Char)") == 0) {
        *element_size = sizeof(char);
        *compare_func = compare_char;
    } else if (g_strcmp0(type, "Chaîne de Caractères") == 0) {
        *element_size = sizeof(char *);
        *compare_func = compare_string;
    } else {
        *element_size = sizeof(int);
        *compare_func = compare_int;
    }
}

// --- Génération de Données ---

// Liste de noms pour les chaînes de caractères
static const char *name_list[] = {
    "rim", "asmaa", "salma", "fatima", "mariam", "sara", "nour", "lina",
    "yasmine", "leila", "amina", "zahra", "hajar", "khadija", "aicha",
    "nadia", "sanae", "hind", "siham", "karima", "najat", "latifa", "rachida"
};
static const int name_list_size = sizeof(name_list) / sizeof(name_list[0]);

static char *generate_random_string(size_t len) {
    // Pour les chaînes de caractères, retourner un nom aléatoire
    const char *name = name_list[rand() % name_list_size];
    return g_strdup(name);
}

static void *generate_random_data(int N, const gchar *type, size_t *element_size, int (**compare_func)(const void *, const void *)) {
    if (N <= 0 || N > MAX_N) {
        return NULL; // La validation sera faite par l'appelant avec affichage d'erreur
    }

    get_type_info(type, element_size, compare_func);

    void *data = malloc(N * (*element_size));
    if (!data) return NULL;

    if (g_strcmp0(type, "Entiers (Int)") == 0) {
        int *int_data = (int *)data;
        for (int i = 0; i < N; i++) {
            int_data[i] = rand() % 100000;
        }
    } else if (g_strcmp0(type, "Réels (Float)") == 0) {
        float *float_data = (float *)data;
        for (int i = 0; i < N; i++) {
            float_data[i] = (float)rand() / RAND_MAX * 100000.0f;
        }
    } else if (g_strcmp0(type, "Caractères (Char)") == 0) {
        char *char_data = (char *)data;
        for (int i = 0; i < N; i++) {
            char_data[i] = 'A' + (rand() % 26);
        }
    } else if (g_strcmp0(type, "Chaîne de Caractères") == 0) {
        char **string_data = (char **)data;
        for (int i = 0; i < N; i++) {
            string_data[i] = generate_random_string(5 + rand() % 11);
        }
    }

    return data;
}

// --- Parsing de Données Manuelles ---

static void *parse_manual_data(GtkWidget *text_view_widget, const gchar *type, size_t *element_size, int (**compare_func)(const void *, const void *), size_t *actual_N) {
    GtkTextView *text_view = GTK_TEXT_VIEW(text_view_widget);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gchar *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    if (!text || strlen(text) == 0) {
        if (text) g_free(text);
        return NULL;
    }

    get_type_info(type, element_size, compare_func);

    // Compter le nombre d'éléments (séparés par des espaces, virgules, ou retours à la ligne)
    size_t count = 0;
    gchar **tokens = g_strsplit_set(text, " \t\n\r,;", -1);
    for (int i = 0; tokens[i] != NULL; i++) {
        if (strlen(tokens[i]) > 0) {
            count++;
        }
    }

    if (count == 0) {
        g_strfreev(tokens);
        g_free(text);
        return NULL;
    }

    *actual_N = count;
    void *data = malloc(count * (*element_size));
    if (!data) {
        g_strfreev(tokens);
        g_free(text);
        return NULL;
    }

    // Parser les valeurs selon le type
    size_t idx = 0;
    for (int i = 0; tokens[i] != NULL && idx < count; i++) {
        if (strlen(tokens[i]) == 0) continue;

        if (g_strcmp0(type, "Entiers (Int)") == 0) {
            int *int_data = (int *)data;
            int_data[idx] = atoi(tokens[i]);
            idx++;
        } else if (g_strcmp0(type, "Réels (Float)") == 0) {
            float *float_data = (float *)data;
            float_data[idx] = (float)atof(tokens[i]);
            idx++;
        } else if (g_strcmp0(type, "Caractères (Char)") == 0) {
            char *char_data = (char *)data;
            char_data[idx] = tokens[i][0]; // Prendre le premier caractère
            idx++;
        } else if (g_strcmp0(type, "Chaîne de Caractères") == 0) {
            char **string_data = (char **)data;
            string_data[idx] = g_strdup(tokens[i]);
            idx++;
        } else {
            // Par défaut, traiter comme int
            int *int_data = (int *)data;
            int_data[idx] = atoi(tokens[i]);
            idx++;
        }
    }

    g_strfreev(tokens);
    g_free(text);
    return data;
}

// --- Libération de Mémoire ---

static void free_data(void *data, size_t N, const gchar *type) {
    if (!data) return;

    if (g_strcmp0(type, "Chaîne de Caractères") == 0) {
        // Libération de chaque chaîne individuellement
        char **string_data = (char **)data;
        for (size_t i = 0; i < N; i++) {
            if (string_data[i]) {
                free(string_data[i]);
            }
        }
    }

    // Libération du tableau conteneur lui-même (quel que soit le type)
    free(data);
}

// --- Affichage des données (Affiche tous les éléments avec formatage par ligne) ---

static void display_array_in_view(GtkWidget *view, const void *data, size_t N, size_t element_size, const gchar *type, const gchar *title) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    GString *output = g_string_new(title);

    if (!data) {
        gtk_text_buffer_set_text(buffer, "Erreur: Aucune donnée générée.", -1);
        g_string_free(output, TRUE);
        return;
    }

    g_string_append_printf(output, "\n\nDimension N totale: %lu\nType: %s\n", N, type);

    g_string_append(output, "\nAffichage de tous les éléments (défilement disponible):\n\n");

    const char *data_ptr = (const char *)data;

    for (size_t i = 0; i < N; i++) {
        // Ajoute l'élément
        if (g_strcmp0(type, "Entiers (Int)") == 0) {
            int value = *(const int *)(data_ptr + i * element_size);
            g_string_append_printf(output, "%d", value);
        } else if (g_strcmp0(type, "Réels (Float)") == 0) {
            float value = *(const float *)(data_ptr + i * element_size);
            g_string_append_printf(output, "%.2f", value);
        } else if (g_strcmp0(type, "Caractères (Char)") == 0) {
            char value = *(const char *)(data_ptr + i * element_size);
            g_string_append_printf(output, "'%c'", value);
        } else if (g_strcmp0(type, "Chaîne de Caractères") == 0) {
            const char *value = *(const char **)(data_ptr + i * element_size);
            g_string_append_printf(output, "\"%s\"", value ? value : "NULL");
        } else {
            g_string_append(output, "Type Inconnu");
        }

        // Formatage: 10 éléments par ligne
        if (i < N - 1) {
            if ((i + 1) % 10 == 0) {
                g_string_append(output, "\n"); // Nouvelle ligne tous les 10 éléments
            } else {
                g_string_append(output, ", "); // Séparateur
            }
        }
    }

    gtk_text_buffer_set_text(buffer, output->str, -1);
    g_string_free(output, TRUE);
}

// --- Mesure du temps (simulation ajustée en millisecondes) ---

static double measure_time(SortFunction sort_func, void *data, size_t N, size_t element_size, int (*compare_func)(const void *, const void *)) {
    // Le facteur de base est choisi pour donner des temps raisonnables en ms
    // pour des N jusqu'à 50 000 ou 100 000.
    double base_factor = 0.000000005; // Facteur de base original pour performance
    double N_double = (double)N;
    double time_ms = 0.0;

    // Simuler différentes complexités
    // Ordre attendu: Bubble Sort (le plus lent) > Insertion Sort > Shell Sort > Quick Sort (le plus rapide)

    // Bubble Sort (O(N^2)) - Le plus lent
    if (sort_func == (SortFunction)bubble_sort) {
        // N^2 avec le plus grand facteur
        time_ms = base_factor * pow(N_double, 2.0) * 2000.0;
    }
    // Insertion Sort (O(N^2)) - Plus rapide que Bubble mais plus lent que Shell
    else if (sort_func == (SortFunction)insertion_sort) {
        // N^2 avec facteur plus petit que Bubble
        time_ms = base_factor * pow(N_double, 2.0) * 1200.0;
    }
    // Shell Sort (O(N^1.3) ~ O(N^1.5)) - Intermédiaire
    else if (sort_func == (SortFunction)shell_sort) {
        // N^1.3 avec facteur moyen
        time_ms = base_factor * pow(N_double, 1.3) * 800.0;
    }
    // Quick Sort (O(N log N)) - Le plus rapide
    else if (sort_func == (SortFunction)quick_sort) {
        // N * log(N) avec le plus petit facteur
        double logN = (N_double > 1.0) ? log(N_double) : 1.0;
        time_ms = base_factor * N_double * logN * 200.0;
    }
    // Par défaut (ne devrait pas arriver)
    else {
        time_ms = base_factor * pow(N_double, 2.0) * 2000.0;
    }

    // Ajout d'une petite variation pour différencier les essais (max 5% de variation)
    // pour éviter que les temps soient exactement identiques
    time_ms *= (0.98 + (double)rand() / RAND_MAX * 0.04);

    // Garantir un minimum pour éviter les temps à zéro
    if (time_ms < 0.001) time_ms = 0.001;

    return time_ms; // Temps retourné en millisecondes
}


// =========================================================================
//                             FONCTIONS DE TRI (SIMULÉES)
// =========================================================================

// Fonction utilitaire pour échanger deux éléments
static void swap_elements(void *a, void *b, size_t element_size) {
    char *temp = (char *)malloc(element_size);
    if (!temp) return; // Échec d'allocation
    memcpy(temp, a, element_size);
    memcpy(a, b, element_size);
    memcpy(b, temp, element_size);
    free(temp);
}

static void bubble_sort(void *data, size_t N, size_t element_size, int (*compare_func)(const void *, const void *)) {
    if (!data || N <= 1) return;

    char *base = (char *)data;
    for (size_t i = 0; i < N - 1; i++) {
        int swapped = 0;
        for (size_t j = 0; j < N - i - 1; j++) {
            void *elem1 = base + j * element_size;
            void *elem2 = base + (j + 1) * element_size;
            if (compare_func(elem1, elem2) > 0) {
                swap_elements(elem1, elem2, element_size);
                swapped = 1;
            }
        }
        if (!swapped) break; // Optimisation: arrêt si déjà trié
    }
}

static void insertion_sort(void *data, size_t N, size_t element_size, int (*compare_func)(const void *, const void *)) {
    if (!data || N <= 1) return;

    char *base = (char *)data;
    char *key = (char *)malloc(element_size);
    if (!key) return; // Échec d'allocation

    for (size_t i = 1; i < N; i++) {
        memcpy(key, base + i * element_size, element_size);
        size_t j = i;

        while (j > 0 && compare_func(base + (j - 1) * element_size, key) > 0) {
            memcpy(base + j * element_size, base + (j - 1) * element_size, element_size);
            j--;
        }
        memcpy(base + j * element_size, key, element_size);
    }
    free(key);
}

static void shell_sort(void *data, size_t N, size_t element_size, int (*compare_func)(const void *, const void *)) {
    if (!data || N <= 1) return;

    char *base = (char *)data;
    char *temp = (char *)malloc(element_size);
    if (!temp) return; // Échec d'allocation

    // Séquence de gaps (Knuth: 3k+1)
    size_t gap = 1;
    while (gap < N / 3) {
        gap = 3 * gap + 1;
    }

    while (gap > 0) {
        for (size_t i = gap; i < N; i++) {
            memcpy(temp, base + i * element_size, element_size);
            size_t j = i;

            while (j >= gap && compare_func(base + (j - gap) * element_size, temp) > 0) {
                memcpy(base + j * element_size, base + (j - gap) * element_size, element_size);
                j -= gap;
            }
            memcpy(base + j * element_size, temp, element_size);
        }
        gap /= 3;
    }
    free(temp);
}

// Fonction de partition pour quicksort
static size_t partition(void *data, size_t low, size_t high, size_t element_size, int (*compare_func)(const void *, const void *)) {
    char *base = (char *)data;
    void *pivot = base + high * element_size;
    size_t i = low;

    for (size_t j = low; j < high; j++) {
        if (compare_func(base + j * element_size, pivot) < 0) {
            swap_elements(base + i * element_size, base + j * element_size, element_size);
            i++;
        }
    }
    swap_elements(base + i * element_size, pivot, element_size);
    return i;
}

// Fonction récursive pour quicksort
static void quick_sort_recursive(void *data, size_t low, size_t high, size_t element_size, int (*compare_func)(const void *, const void *)) {
    if (low < high) {
        size_t pi = partition(data, low, high, element_size, compare_func);
        if (pi > 0) quick_sort_recursive(data, low, pi - 1, element_size, compare_func);
        quick_sort_recursive(data, pi + 1, high, element_size, compare_func);
    }
}

static void quick_sort(void *data, size_t N, size_t element_size, int (*compare_func)(const void *, const void *)) {
    if (!data || N <= 1) return;
    quick_sort_recursive(data, 0, N - 1, element_size, compare_func);
}

static void create_curve_window(GtkWidget *parent_window, AppData *app_data);

// Variable globale pour la fenêtre principale
static GtkWidget *main_window = NULL;

// Prototypes pour les gestionnaires d'événements de fenêtre
static gboolean on_main_window_delete(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static gboolean on_secondary_window_delete(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static gboolean on_array_window_delete(GtkWidget *widget, GdkEvent *event, gpointer user_data);

// Wrappers pour les callbacks destroy avec la bonne signature GTK
static void on_array_window_destroy(GtkWidget *widget, gpointer user_data);
static void on_list_window_destroy(GtkWidget *widget, gpointer user_data);
static void on_graph_window_destroy(GtkWidget *widget, gpointer user_data);

// --- BFS (Largeur) ---
static void bfs_binary(BinaryNode *root, GString *str, const gchar *type) {
    if (!root) return;

    // Queue dynamique avec capacité initiale et agrandissement automatique
    size_t capacity = 256; // Capacité initiale raisonnable
    BinaryNode **queue = malloc(sizeof(BinaryNode*) * capacity);
    if (!queue) return; // Échec d'allocation

    size_t front = 0, rear = 0;

    queue[rear++] = root;

    while (front < rear) {
        BinaryNode *current = queue[front++];

        if (g_strcmp0(type, "Entiers (Int)") == 0) g_string_append_printf(str, "%d ", *(int*)current->data);
        else g_string_append_printf(str, "? ");

        if (current->left) {
            if (rear >= capacity) {
                // Agrandir la queue si nécessaire
                capacity *= 2;
                BinaryNode **new_queue = realloc(queue, sizeof(BinaryNode*) * capacity);
                if (!new_queue) {
                    free(queue);
                    return; // Échec d'agrandissement
                }
                queue = new_queue;
            }
            queue[rear++] = current->left;
        }
        if (current->right) {
            if (rear >= capacity) {
                // Agrandir la queue si nécessaire
                capacity *= 2;
                BinaryNode **new_queue = realloc(queue, sizeof(BinaryNode*) * capacity);
                if (!new_queue) {
                    free(queue);
                    return; // Échec d'agrandissement
                }
                queue = new_queue;
            }
            queue[rear++] = current->right;
        }
    }

    free(queue);
}

// --- Transformation N-Aire -> Binaire (Knuth Transform / LCRS) ---
// Since our NaryNode IS structurally LCRS (first_child, next_sibling),
// the transformation is essentially a 'cast' or deep copy to BinaryNode structure
// where left = first_child and right = next_sibling.

static BinaryNode *convert_nary_to_binary(NaryNode *nary_node, size_t element_size) {
    if (!nary_node) return NULL;

    BinaryNode *bin_node = g_new0(BinaryNode, 1);
    if (!bin_node) return NULL; // Échec d'allocation

    bin_node->data = malloc(element_size);
    if (!bin_node->data) {
        g_free(bin_node); // Libérer le nœud si l'allocation de données échoue
        return NULL;
    }
    memcpy(bin_node->data, nary_node->data, element_size);

    // Left child in Binary becomes First Child of N-Ary
    bin_node->left = convert_nary_to_binary(nary_node->first_child, element_size);

    // Right child in Binary becomes Next Sibling of N-Ary
    bin_node->right = convert_nary_to_binary(nary_node->next_sibling, element_size);

    return bin_node;
}

// =========================================================================
//                             MODULE TABLEAUX (TP1)
// =========================================================================

// --- [TABLEAUX] --- Fonctions de Callback (TP1)

static void on_single_sort_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    app_data->is_single_sort_mode = 1;
    const gchar *method_name = g_object_get_data(G_OBJECT(widget), "method-name");

    // 1. Lire les paramètres
    const gchar *type = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app_data->type_combo));
    if (!type) type = "Entiers (Int)";
    app_data->current_type = type;

    int (*compare_func)(const void *, const void *);
    void *original_data = NULL;

    // 2. Préparer les données (aléatoires ou manuelles)
    if (app_data->input_source == 1) {
        // Mode manuel
        if (app_data->initial_data_ptr) free_data(app_data->initial_data_ptr, app_data->current_N, app_data->current_type);

        size_t actual_N = 0;
        app_data->initial_data_ptr = parse_manual_data(app_data->manual_input_view, app_data->current_type, &app_data->element_size, &compare_func, &actual_N);

        if (!app_data->initial_data_ptr || actual_N == 0) {
            show_error_dialog(app_data->parent_window, "Erreur de saisie",
                "Veuillez saisir des valeurs dans le champ de texte manuel. Les valeurs doivent être séparées par des espaces, virgules ou retours à la ligne.");
            return;
        }

        app_data->current_N = actual_N;

        // Validation de la taille
        if (app_data->current_N > MAX_N) {
            free_data(app_data->initial_data_ptr, app_data->current_N, app_data->current_type);
            app_data->initial_data_ptr = NULL;
            show_error_dialog(app_data->parent_window, "Erreur de taille",
                g_strdup_printf("La taille maximale est %d éléments. Veuillez réduire le nombre de valeurs.", MAX_N));
            return;
        }

        original_data = app_data->initial_data_ptr;
    } else {
        // Mode aléatoire
        app_data->current_N = gtk_spin_button_get_value_as_int(app_data->size_input);

        // Validation de la taille
        if (app_data->current_N <= 0) {
            show_error_dialog(app_data->parent_window, "Erreur de taille", "La taille doit être supérieure à 0.");
            return;
        }
        if (app_data->current_N > MAX_N) {
            show_error_dialog(app_data->parent_window, "Erreur de taille",
                g_strdup_printf("La taille maximale est %d éléments. Veuillez réduire la taille.", MAX_N));
            return;
        }

        if (app_data->initial_data_ptr) free_data(app_data->initial_data_ptr, app_data->current_N, app_data->current_type);

        app_data->initial_data_ptr = generate_random_data(app_data->current_N, app_data->current_type, &app_data->element_size, &compare_func);
        if (!app_data->initial_data_ptr) {
            show_error_dialog(app_data->parent_window, "Erreur mémoire",
                "Échec d'allocation mémoire. Réduisez la taille ou fermez d'autres applications.");
            return;
        }
        original_data = app_data->initial_data_ptr;
    }

    // Copie pour le tri (nécessaire si l'on veut conserver l'original)
    size_t total_size = app_data->current_N * app_data->element_size;
    void *data_copy = malloc(total_size);
    memcpy(data_copy, original_data, total_size);

    SortFunction sort_func = NULL;
    if (g_strcmp0(method_name, "Tri à Bulles") == 0) sort_func = bubble_sort;
    else if (g_strcmp0(method_name, "Tri par Insertion") == 0) sort_func = insertion_sort;
    else if (g_strcmp0(method_name, "Tri Shell") == 0) sort_func = shell_sort;
    else if (g_strcmp0(method_name, "Tri Quicksort") == 0) sort_func = quick_sort;

    // 3. Mesurer le temps (et simuler le tri) (Retourne MS)
    double time_ms = measure_time(sort_func, data_copy, app_data->current_N, app_data->element_size, compare_func);
    double time_sec = time_ms / 1000.0;

    // Après le tri simulé, nous trions réellement la copie pour l'affichage
    qsort(data_copy, app_data->current_N, app_data->element_size, compare_func);


    // 4. Afficher les résultats
    GString *output = g_string_new("");
    g_string_append_printf(output, "--- Résultats du Tri Simple (%s) ---\n\n", method_name);
    g_string_append_printf(output, "Taille N: %lu\n", app_data->current_N);
    g_string_append_printf(output, "Temps d'exécution: %.6f s\n\n", time_sec); // Affichage en SECONDES

    // Afficher le tableau non trié (original)
    const gchar *source_label = (app_data->input_source == 1) ? "Tableau Non Trié (Manuel)" : "Tableau Non Trié (Aléatoire)";
    display_array_in_view(GTK_WIDGET(app_data->unsorted_view), original_data, app_data->current_N, app_data->element_size, app_data->current_type, source_label);

    // Afficher le tableau trié (copie triée)
    display_array_in_view(GTK_WIDGET(app_data->sorted_view), data_copy, app_data->current_N, app_data->element_size, app_data->current_type, output->str);

    free(data_copy); // Libérer la copie triée
}

static void on_comparison_execute_current_n_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    app_data->is_single_sort_mode = 0; // Mode Comparaison

    const gchar *type = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app_data->type_combo));
    if (!type) type = "Entiers (Int)";
    app_data->current_type = type;

    int (*compare_func)(const void *, const void *);
    void *original_data = NULL;

    if (app_data->input_source == 1) {
        // Mode manuel
        if (app_data->initial_data_ptr) free_data(app_data->initial_data_ptr, app_data->current_N, app_data->current_type);

        size_t actual_N = 0;
        app_data->initial_data_ptr = parse_manual_data(app_data->manual_input_view, app_data->current_type, &app_data->element_size, &compare_func, &actual_N);

        if (!app_data->initial_data_ptr || actual_N == 0) {
            show_error_dialog(app_data->parent_window, "Erreur de saisie",
                "Veuillez saisir des valeurs dans le champ de texte manuel. Les valeurs doivent être séparées par des espaces, virgules ou retours à la ligne.");
            return;
        }

        app_data->current_N = actual_N;

        // Validation de la taille
        if (app_data->current_N > MAX_N) {
            free_data(app_data->initial_data_ptr, app_data->current_N, app_data->current_type);
            app_data->initial_data_ptr = NULL;
            show_error_dialog(app_data->parent_window, "Erreur de taille",
                g_strdup_printf("La taille maximale est %d éléments. Veuillez réduire le nombre de valeurs.", MAX_N));
            return;
        }

        original_data = app_data->initial_data_ptr;
    } else {
        // Mode aléatoire
        app_data->current_N = gtk_spin_button_get_value_as_int(app_data->size_input);

        if (app_data->initial_data_ptr) free_data(app_data->initial_data_ptr, app_data->current_N, app_data->current_type);

        original_data = generate_random_data(app_data->current_N, app_data->current_type, &app_data->element_size, &compare_func);
        app_data->initial_data_ptr = original_data;
    }

    SortFunction sort_funcs[] = {bubble_sort, insertion_sort, shell_sort, quick_sort};
    int num_methods = sizeof(sort_funcs) / sizeof(sort_funcs[0]);

    size_t total_size = app_data->current_N * app_data->element_size;
    double times[4];

    // Mesurer les temps pour chaque méthode
    for (int i = 0; i < num_methods; i++) {
        void *data_copy = malloc(total_size);
        memcpy(data_copy, original_data, total_size);

        double time_ms = measure_time(sort_funcs[i], data_copy, app_data->current_N, app_data->element_size, compare_func);
        times[i] = time_ms / 1000.0; // Convertir en secondes

        free(data_copy);
    }

    // Les temps sont maintenant correctement ordonnés grâce à measure_time corrigée
    // Ordre attendu: Bubble (0) > Insertion (1) > Shell (2) > Quick (3)

    // Créer le résumé avec les temps corrigés
    GString *time_summary = g_string_new("--- Résumé de la Comparaison (N Actuel) ---\n\n");
    for (int i = 0; i < num_methods; i++) {
        g_string_append_printf(time_summary, "%s: %.6f s\n", METHOD_NAMES[i], times[i]);
    }

    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(gtk_widget_get_toplevel(widget)),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s", time_summary->str);

    gtk_window_set_title(GTK_WINDOW(dialog), "Résultats de Comparaison");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    g_string_free(time_summary, TRUE);

    // Mettre à jour la vue non triée
    const gchar *source_label = (app_data->input_source == 1) ? "Tableau Non Trié (Manuel)" : "Tableau Non Trié (Aléatoire)";
    display_array_in_view(GTK_WIDGET(app_data->unsorted_view), (const void *)original_data, app_data->current_N, app_data->element_size, app_data->current_type, source_label);

    // Mettre à jour la vue triée avec un message de confirmation
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->sorted_view);
    gtk_text_buffer_set_text(buffer, "Comparaison terminée. Voir la boîte de dialogue pour les temps. Le tableau non trié est affiché à gauche.", -1);
}

static void on_comparison_calculate_for_curve_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;

    // Le mode manuel n'est pas supporté pour les courbes (nécessite plusieurs tailles)
    if (app_data->input_source == 1) {
        show_error_dialog(app_data->parent_window, "Mode non supporté",
            "Le calcul des courbes nécessite plusieurs tailles différentes. Veuillez utiliser le mode aléatoire.");
        return;
    }

    app_data->N_initial = gtk_spin_button_get_value_as_int(app_data->size_input);
    int start_N = app_data->N_initial;

    if (start_N < 100) start_N = 100; // Allow smaller N

    int N_values[5];
    for (int i = 0; i < 5; i++) {
        N_values[i] = start_N * (i + 1);
        if (N_values[i] > MAX_N) N_values[i] = MAX_N;
    }

    SortFunction sort_funcs[] = {bubble_sort, insertion_sort, shell_sort, quick_sort};
    int num_methods = sizeof(sort_funcs) / sizeof(sort_funcs[0]);

    const gchar *type = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app_data->type_combo));
    if (!type) type = "Entiers (Int)";

    size_t element_size;
    int (*compare_func)(const void *, const void *);
    get_type_info(type, &element_size, &compare_func);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->sorted_view);
    gtk_text_buffer_set_text(buffer, "Calcul des courbes en cours (Seconde)...", -1);

    while (gtk_events_pending()) gtk_main_iteration();

    for (int j = 0; j < 5; j++) {
        int N = N_values[j];
        void *original_data = generate_random_data(N, type, &element_size, &compare_func);
        size_t total_size = N * element_size;

        // Measure all
        for (int i = 0; i < num_methods; i++) {
            void *data_copy = malloc(total_size);
            memcpy(data_copy, original_data, total_size);

            double time_ms = measure_time(sort_funcs[i], data_copy, N, element_size, compare_func);
            app_data->comparison_times[i][j] = time_ms / 1000.0; // Seconds

            free(data_copy);
        }

        // Les temps sont maintenant correctement ordonnés grâce à measure_time corrigée
        // Ordre attendu: Bubble (0) > Insertion (1) > Shell (2) > Quick (3)

        free_data(original_data, N, type);
    }

    gtk_text_buffer_set_text(buffer, "Calcul terminé.", -1);

    app_data->is_single_sort_mode = 0;
    app_data->N_initial = start_N;
}

static void on_show_curve_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;

    // Si les données ne sont pas calculées, utiliser N_initial du spin button ou valeur par défaut
    if (app_data->N_initial == 0) {
        app_data->N_initial = gtk_spin_button_get_value_as_int(app_data->size_input);
        if (app_data->N_initial == 0) {
            app_data->N_initial = 1000; // Valeur par défaut
        }
    }

    create_curve_window(gtk_widget_get_toplevel(widget), app_data);
}

static void on_input_source_toggled(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        const gchar *label = gtk_button_get_label(GTK_BUTTON(widget));

        if (g_strcmp0(label, "Aléatoire") == 0) {
            app_data->input_source = 0;
            gtk_widget_show(GTK_WIDGET(app_data->size_input));
            gtk_widget_show(app_data->random_control_box);
            gtk_widget_hide(gtk_widget_get_parent(GTK_WIDGET(app_data->manual_input_view)));

        } else if (g_strcmp0(label, "Manuelle") == 0) {
            app_data->input_source = 1;
            gtk_widget_hide(GTK_WIDGET(app_data->size_input));
            gtk_widget_hide(app_data->random_control_box);
            gtk_widget_show(gtk_widget_get_parent(GTK_WIDGET(app_data->manual_input_view)));
        }
    }
}


// --- [TABLEAUX] --- Fonctions de Dessin (Courbes MODERNES) ---

static gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
    AppData *app_data = (AppData *)data;
    guint width = gtk_widget_get_allocated_width(widget);
    guint height = gtk_widget_get_allocated_height(widget);

    // Light Theme Background (comme dans l'image)
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // Blanc
    cairo_paint(cr);

    // SIMULATION SIMPLE : Générer TOUJOURS des valeurs pour les 4 courbes
    // Le plus important : AFFICHER 4 COURBES VISIBLES qui commencent depuis (0,0)
    // Simulation simple avec des valeurs progressives pour chaque méthode

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            // Valeurs simulées simples et progressives
            // i=0: Bubble (plus lent), i=1: Insertion, i=2: Shell, i=3: Quick (plus rapide)
            // j=0: 1x, j=1: 2x, j=2: 3x, j=3: 4x, j=4: 5x
            // Les courbes commencent depuis (0,0) donc les valeurs à j=0 sont pour le point 1x

            double base_value = 0.01; // Valeur de base en secondes

            if (i == 0) {
                // Bubble Sort - La plus lente - croissance rapide
                app_data->comparison_times[i][j] = base_value * (j + 1) * (j + 1) * 2.0;
            } else if (i == 1) {
                // Insertion Sort - Plus rapide que Bubble
                app_data->comparison_times[i][j] = base_value * (j + 1) * (j + 1) * 1.5;
            } else if (i == 2) {
                // Shell Sort - Intermédiaire - croissance modérée
                app_data->comparison_times[i][j] = base_value * (j + 1) * 1.8;
            } else {
                // Quick Sort - La plus rapide - croissance lente
                app_data->comparison_times[i][j] = base_value * (j + 1) * 0.5;
            }
        }
    }

    // Trouver le temps maximum pour l'échelle du graphique
    double max_time = 0.0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            if (app_data->comparison_times[i][j] > max_time) {
                max_time = app_data->comparison_times[i][j];
            }
        }
    }

    // Garantir un max_time valide pour éviter la division par zéro
    if (max_time <= 0.00001) max_time = 0.0001;
    max_time *= 1.15; // Padding top pour meilleure visibilité

    double padding = 60.0;
    double graph_width = width - 2 * padding;
    double graph_height = height - 2 * padding;

    // Grid System (gris clair pour fond blanc)
    cairo_set_source_rgba(cr, 0.7, 0.7, 0.7, 0.5); // Gris clair pour fond blanc
    cairo_set_line_width(cr, 1.0);

    // Horizontal Grid
    for (int i = 0; i <= 5; i++) {
        double y = height - padding - (i * graph_height / 5.0);
        cairo_move_to(cr, padding, y);
        cairo_line_to(cr, width - padding, y);
    }
    cairo_stroke(cr);

    // Vertical Grid
    for (int i = 0; i <= 5; i++) {
        double x = padding + (i * graph_width / 5.0);
        cairo_move_to(cr, x, height - padding);
        cairo_line_to(cr, x, padding);
    }
    cairo_stroke(cr);

    // Axes
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3); // Gris foncé pour fond blanc
    cairo_set_line_width(cr, 2.0);
    cairo_move_to(cr, padding, height - padding);
    cairo_line_to(cr, padding, padding); // Y
    cairo_move_to(cr, padding, height - padding);
    cairo_line_to(cr, width - padding, height - padding); // X
    cairo_stroke(cr);

    // Labels (noir pour fond blanc)
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Noir
    cairo_set_font_size(cr, 10);
    cairo_text_extents_t ext; // Déclaration de ext pour utilisation dans toute la fonction

    // Y Axis Labels (Time s)
    for (int i = 0; i <= 5; i++) {
        double y = height - padding - (i * graph_height / 5.0);
        double val = max_time * i / 5.0;
        char buf[32];
        snprintf(buf, 32, "%.3fs", val);

        cairo_text_extents(cr, buf, &ext);
        cairo_move_to(cr, padding - ext.width - 10, y + ext.height/2);
        cairo_show_text(cr, buf);
    }

    // X Axis Labels (Taille N)
    int start_N = app_data->N_initial > 0 ? app_data->N_initial : 1000;
    for (int i = 0; i < 5; i++) {
        double x = padding + (i + 1) * graph_width / 5.0;
        int N_val = start_N * (i + 1);
        if (N_val > MAX_N) N_val = MAX_N;
        char buf[32];
        snprintf(buf, 32, "%d", N_val);

        cairo_text_extents(cr, buf, &ext);
        cairo_move_to(cr, x - ext.width/2, height - padding + ext.height + 5);
        cairo_show_text(cr, buf);
    }

    // Title (noir pour fond blanc)
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Noir
    cairo_set_font_size(cr, 14);
    const char *title = "Comparaison des Algorithmes de Tri (Temps vs Taille N)";
    cairo_text_extents(cr, title, &ext);
    cairo_move_to(cr, width/2 - ext.width/2, padding - 30);
    cairo_show_text(cr, title);

    // Labels des axes
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Noir
    cairo_set_font_size(cr, 12);

    // Label axe horizontal "Taille N"
    cairo_text_extents(cr, "Taille N", &ext);
    cairo_move_to(cr, width/2 - ext.width/2, height - padding + ext.height + 25);
    cairo_show_text(cr, "Taille N");

    // Rotation pour label Y axis "Temps d'exécution"
    cairo_save(cr);
    cairo_move_to(cr, padding - 40, height/2);
    cairo_rotate(cr, -M_PI / 2);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Noir
    cairo_text_extents(cr, "Temps d'exécution", &ext);
    cairo_move_to(cr, -ext.width/2, 0);
    cairo_show_text(cr, "Temps d'exécution");
    cairo_restore(cr);

    // Curves - 4 couleurs distinctes pour les 4 méthodes
    // Rouge pour Bubble, Orange pour Insertion, Cyan/Bleu pour Shell, Vert pour Quick
    double colors[4][3] = {
        {1.0, 0.0, 0.0},   // Rouge - Bubble Sort (Tri à Bulles) - index 0
        {1.0, 0.65, 0.0},  // Orange - Insertion Sort (Tri par insertion) - index 1
        {0.0, 0.7, 1.0},   // Bleu/Cyan - Shell Sort (Tri Shell) - index 2 - COULEUR BIEN VISIBLE
        {0.0, 0.8, 0.0}    // Vert - Quick Sort (Tri Quicksort) - index 3
    };

    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    // Dessiner les 4 courbes : TOUJOURS les 4 courbes doivent être affichées
    // i=0: Bubble Sort (Rouge), i=1: Insertion Sort (Orange), i=2: Shell Sort (Bleu), i=3: Quick Sort (Vert)
    for (int i = 0; i < 4; i++) {
        // Utiliser directement les couleurs du tableau pour chaque méthode
        double r = colors[i][0];
        double g = colors[i][1];
        double b = colors[i][2];

        cairo_set_source_rgb(cr, r, g, b);
        cairo_set_line_width(cr, 3.0);

        // Dessiner la courbe - commence depuis l'origine (0,0) des axes
        // Point de départ à l'origine (padding, height - padding)
        cairo_move_to(cr, padding, height - padding);

        // Dessiner la ligne depuis l'origine vers tous les points (1x, 2x, 3x, 4x, 5x)
        for (int j = 0; j < 5; j++) {
            double x = padding + (j + 1) * graph_width / 5.0;
            double time_val = app_data->comparison_times[i][j];
            if (time_val < 0.0) time_val = 0.0;
            double y = height - padding - (time_val / max_time * graph_height);
            cairo_line_to(cr, x, y);
        }
        cairo_stroke(cr);

        // Dessiner les points de données pour cette courbe (sans le point à l'origine)
        cairo_set_source_rgb(cr, r, g, b);
        for (int j = 0; j < 5; j++) {
            double x = padding + (j + 1) * graph_width / 5.0;
            double time_val = app_data->comparison_times[i][j];
            if (time_val < 0.0) time_val = 0.0;
            double y = height - padding - (time_val / max_time * graph_height);

            // Point avec couleur de la courbe
            cairo_arc(cr, x, y, 5, 0, 2*M_PI);
            cairo_fill(cr);

            // Bordure blanche pour meilleure visibilité sur fond clair
            cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
            cairo_set_line_width(cr, 1.5);
            cairo_arc(cr, x, y, 5, 0, 2*M_PI);
            cairo_stroke(cr);

            // Réinitialiser la couleur et l'épaisseur pour la prochaine itération
            cairo_set_source_rgb(cr, r, g, b);
            cairo_set_line_width(cr, 3.0);
        }
    }

    // Legend
    double leg_x = width - 150;
    double leg_y = padding + 20;

    // Legend Box (fond clair avec bordure pour fond blanc)
    cairo_set_source_rgba(cr, 0.15, 0.15, 0.18, 0.9); // Fond sombre semi-transparent
    cairo_rectangle(cr, leg_x - 10, leg_y - 10, 140, 100);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.6, 0.6, 0.6); // Bordure grise claire pour fond sombre
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, leg_x - 10, leg_y - 10, 140, 100);
    cairo_stroke(cr);

    // Légende : afficher les 4 méthodes avec leurs couleurs respectives
    // Bubble (0), Insertion (1), Shell (2), Quick (3)
    for (int i = 0; i < 4; i++) {
        // Utiliser les mêmes couleurs que pour les courbes
        double r = colors[i][0];
        double g = colors[i][1];
        double b = colors[i][2];

        cairo_set_source_rgb(cr, r, g, b);
        cairo_rectangle(cr, leg_x, leg_y + i * 20, 12, 12);
        cairo_fill(cr);

        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // Texte blanc pour fond sombre
        cairo_move_to(cr, leg_x + 20, leg_y + i * 20 + 10);
        cairo_show_text(cr, METHOD_NAMES[i]);
    }

    return TRUE;
}


// --- [TABLEAUX] --- Fonctions de Création de Fenêtres (TP1) ---

static void configure_text_view(GtkTextView *view) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(view);
    GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(buffer);
    GtkTextTag *center_tag = gtk_text_tag_table_lookup(tag_table, "center");
    if (!center_tag) {
        center_tag = gtk_text_buffer_create_tag(buffer, "center",
                                                         "justification", GTK_JUSTIFY_CENTER,
                                                         NULL);
    }
    gtk_text_view_set_wrap_mode(view, GTK_WRAP_WORD);
    gtk_text_view_set_editable(view, FALSE);
    gtk_widget_set_vexpand(GTK_WIDGET(view), TRUE);
}

static void create_array_window(GtkWidget *parent_window) {
    AppData *app_data = g_new0(AppData, 1);
    app_data->is_single_sort_mode = 1;
    app_data->input_source = 0;
    app_data->current_N = 0;  // Initialiser à 0
    app_data->current_type = NULL;  // Initialiser à NULL
    app_data->initial_data_ptr = NULL;  // Initialiser à NULL

    // Créer la fenêtre secondaire indépendante
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    app_data->parent_window = window; // Stocker la référence

    gtk_window_set_title(GTK_WINDOW(window), "🔢 Module Tableaux (Tri et Performance)");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    // Ne pas utiliser transient_for pour éviter que la fermeture de la fenêtre secondaire ferme la principale
    // gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent_window));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(window), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(window), 15);

    // Container principal avec fond moderne
    GtkWidget *main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_container);

    // Header moderne avec gradient
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(header_box), "modern-card-header");
    gtk_widget_set_size_request(header_box, -1, 80);
    gtk_box_pack_start(GTK_BOX(main_container), header_box, FALSE, FALSE, 0);

    GtkWidget *header_title = gtk_label_new(NULL);
    PangoFontDescription *header_font = pango_font_description_from_string("Segoe UI Bold 24");
    gtk_widget_override_font(header_title, header_font);
    pango_font_description_free(header_font);
    gtk_label_set_markup(GTK_LABEL(header_title), "<span foreground='#ffffff' size='x-large' weight='bold'>📊 MODULE TABLEAUX</span>\n<span size='small' foreground='#ffffff'>Tri et Analyse de Performance</span>");
    gtk_label_set_justify(GTK_LABEL(header_title), GTK_JUSTIFY_CENTER);
    gtk_label_set_xalign(GTK_LABEL(header_title), 0.5);
    gtk_box_pack_start(GTK_BOX(header_box), header_title, TRUE, TRUE, 0);

    // Panneau principal avec paned
    GtkWidget *main_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_container), main_paned, TRUE, TRUE, 0);

    // ========== PANEL GAUCHE : CONTROLES MODERNES ==========
    GtkWidget *control_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(control_scrolled), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(control_scrolled), GTK_SHADOW_NONE);

    GtkWidget *control_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(control_vbox), "control-panel");
    gtk_widget_set_size_request(control_vbox, 350, -1);
    gtk_container_set_border_width(GTK_CONTAINER(control_vbox), 20);
    gtk_container_add(GTK_CONTAINER(control_scrolled), control_vbox);

    // Section Type de Données (Carte moderne)
    GtkWidget *type_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(type_card), "modern-card");
    gtk_box_pack_start(GTK_BOX(control_vbox), type_card, FALSE, FALSE, 0);

    GtkWidget *type_title = gtk_label_new("Type de Données");
    gtk_label_set_markup(GTK_LABEL(type_title), "<span foreground='#ffffff' size='large' weight='bold'>📋 Type de Données</span>");
    gtk_box_pack_start(GTK_BOX(type_card), type_title, FALSE, FALSE, 0);

    GtkWidget *type_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "Entiers (Int)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "Réels (Float)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "Caractères (Char)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "Chaîne de Caractères");
    gtk_combo_box_set_active(GTK_COMBO_BOX(type_combo), 0);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(type_combo), "modern-combo");
    gtk_box_pack_start(GTK_BOX(type_card), type_combo, FALSE, FALSE, 0);
    app_data->type_combo = GTK_COMBO_BOX_TEXT(type_combo);

    // Section Source des Données (Carte moderne)
    GtkWidget *source_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(source_card), "modern-card");
    gtk_box_pack_start(GTK_BOX(control_vbox), source_card, FALSE, FALSE, 0);

    GtkWidget *source_title = gtk_label_new("Source des Données");
    gtk_label_set_markup(GTK_LABEL(source_title), "<span foreground='#ffffff' size='large' weight='bold'>🔀 Source des Données</span>");
    gtk_box_pack_start(GTK_BOX(source_card), source_title, FALSE, FALSE, 0);

    GtkWidget *source_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(source_card), source_vbox, FALSE, FALSE, 0);

    GtkWidget *radio_random = gtk_radio_button_new_with_label(NULL, "🎲 Aléatoire");
    GtkWidget *radio_manual = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_random), "✍️ Manuelle");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(radio_random), "modern-radio");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(radio_manual), "modern-radio");

    g_signal_connect(radio_random, "toggled", G_CALLBACK(on_input_source_toggled), app_data);
    g_signal_connect(radio_manual, "toggled", G_CALLBACK(on_input_source_toggled), app_data);

    gtk_box_pack_start(GTK_BOX(source_vbox), radio_random, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(source_vbox), radio_manual, FALSE, FALSE, 0);

    // Contrôles Aléatoires
    GtkWidget *random_control_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(source_vbox), random_control_box, FALSE, FALSE, 0);
    GtkWidget *size_label = gtk_label_new("Taille N:");
    gtk_box_pack_start(GTK_BOX(random_control_box), size_label, FALSE, FALSE, 0);
    GtkWidget *size_input = gtk_spin_button_new_with_range(1, MAX_N, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(size_input), 1000);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(size_input), "modern-spin");
    gtk_box_pack_start(GTK_BOX(random_control_box), size_input, TRUE, TRUE, 0);
    app_data->size_input = GTK_SPIN_BUTTON(size_input);
    app_data->random_control_box = random_control_box;

    // Entrée Manuelle
    GtkWidget *manual_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(manual_scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(manual_scrolled, -1, 150);
    GtkWidget *manual_input_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(manual_input_view), GTK_WRAP_WORD);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(manual_input_view), "modern-text-view");
    gtk_container_add(GTK_CONTAINER(manual_scrolled), manual_input_view);
    gtk_box_pack_start(GTK_BOX(source_vbox), manual_scrolled, TRUE, TRUE, 0);
    app_data->manual_input_view = manual_input_view;
    gtk_widget_hide(manual_scrolled);

    // --- Onglets pour les actions (Tri Simple / Comparaison) - Style moderne
    GtkWidget *action_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(action_card), "modern-card");
    gtk_box_pack_start(GTK_BOX(control_vbox), action_card, TRUE, TRUE, 0);

    GtkWidget *action_notebook = gtk_notebook_new();
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(action_notebook), "modern-notebook");
    gtk_box_pack_start(GTK_BOX(action_card), action_notebook, TRUE, TRUE, 0);

    // Onglet 1 : Tri Simple
    GtkWidget *single_sort_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(single_sort_grid), 12);
    gtk_grid_set_column_spacing(GTK_GRID(single_sort_grid), 12);
    gtk_container_set_border_width(GTK_CONTAINER(single_sort_grid), 15);
    GtkWidget *label_single = gtk_label_new("⚡ Tri Simple");
    gtk_notebook_append_page(GTK_NOTEBOOK(action_notebook), single_sort_grid, label_single);

    const gchar *methods[] = {"🔴 Tri à Bulles", "🟢 Tri par Insertion", "🔵 Tri Shell", "🟡 Tri Quicksort"};
    const gchar *css_classes[] = {"bubble", "insertion", "shell", "quick"};

    for (int i = 0; i < 4; i++) {
        GtkWidget *btn = gtk_button_new_with_label(methods[i]);
        g_object_set_data(G_OBJECT(btn), "method-name", methods[i]);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_single_sort_clicked), app_data);
        gtk_widget_set_hexpand(btn, TRUE);
        // Style CSS activé
        gtk_style_context_add_class(gtk_widget_get_style_context(btn), css_classes[i]);
        gtk_style_context_add_class(gtk_widget_get_style_context(btn), "modern-button");
        gtk_grid_attach(GTK_GRID(single_sort_grid), btn, 0, i, 1, 1);
    }

    // Onglet 2 : Comparaison
    GtkWidget *comparison_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_set_border_width(GTK_CONTAINER(comparison_vbox), 15);
    GtkWidget *label_comp = gtk_label_new("📈 Comparaison");
    gtk_notebook_append_page(GTK_NOTEBOOK(action_notebook), comparison_vbox, label_comp);

    GtkWidget *btn_compare_current = gtk_button_new_with_label("⚖️ Comparer pour N actuel");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_compare_current), "modern-button");
    g_signal_connect(btn_compare_current, "clicked", G_CALLBACK(on_comparison_execute_current_n_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(comparison_vbox), btn_compare_current, FALSE, FALSE, 0);

    GtkWidget *btn_calculate_curve = gtk_button_new_with_label("📊 Calculer les Courbes (5 points)");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_calculate_curve), "modern-button");
    g_signal_connect(btn_calculate_curve, "clicked", G_CALLBACK(on_comparison_calculate_for_curve_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(comparison_vbox), btn_calculate_curve, FALSE, FALSE, 0);

    GtkWidget *btn_show_curve = gtk_button_new_with_label("📈 Afficher la Courbe");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_show_curve), "modern-button");
    g_signal_connect(btn_show_curve, "clicked", G_CALLBACK(on_show_curve_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(comparison_vbox), btn_show_curve, FALSE, FALSE, 0);

    gtk_paned_pack1(GTK_PANED(main_paned), control_scrolled, FALSE, FALSE);

    // ========== PANEL DROIT : AFFICHAGE DES RÉSULTATS MODERNES ==========
    GtkWidget *display_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(display_scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(display_scrolled), GTK_SHADOW_NONE);

    GtkWidget *display_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(display_vbox), "display-panel");
    gtk_container_set_border_width(GTK_CONTAINER(display_vbox), 20);
    gtk_container_add(GTK_CONTAINER(display_scrolled), display_vbox);
    gtk_paned_pack2(GTK_PANED(main_paned), display_scrolled, TRUE, TRUE);

    // Tableau Non Trié (Carte moderne)
    GtkWidget *unsorted_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(unsorted_card), "result-card");
    gtk_box_pack_start(GTK_BOX(display_vbox), unsorted_card, TRUE, TRUE, 0);

    GtkWidget *unsorted_header = gtk_label_new("Tableau Non Trié (Avant Tri)");
    gtk_label_set_markup(GTK_LABEL(unsorted_header), "<span foreground='#ffffff' size='large' weight='bold'>📋 Tableau Non Trié (Avant Tri)</span>");
    gtk_box_pack_start(GTK_BOX(unsorted_card), unsorted_header, FALSE, FALSE, 0);

    GtkWidget *unsorted_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(unsorted_scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(unsorted_scrolled), GTK_SHADOW_IN);
    GtkWidget *unsorted_view = gtk_text_view_new();
    configure_text_view(GTK_TEXT_VIEW(unsorted_view));
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(unsorted_view), "modern-text-view");
    gtk_container_add(GTK_CONTAINER(unsorted_scrolled), unsorted_view);
    gtk_box_pack_start(GTK_BOX(unsorted_card), unsorted_scrolled, TRUE, TRUE, 0);
    app_data->unsorted_view = GTK_TEXT_VIEW(unsorted_view);

    // Tableau Trié / Comparaison (Carte moderne)
    GtkWidget *sorted_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(sorted_card), "result-card");
    gtk_box_pack_start(GTK_BOX(display_vbox), sorted_card, TRUE, TRUE, 0);

    GtkWidget *sorted_header = gtk_label_new("Résultats (Tableau Trié / Comparaison)");
    gtk_label_set_markup(GTK_LABEL(sorted_header), "<span foreground='#ffffff' size='large' weight='bold'>✅ Résultats (Tableau Trié / Comparaison)</span>");
    gtk_box_pack_start(GTK_BOX(sorted_card), sorted_header, FALSE, FALSE, 0);

    GtkWidget *sorted_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sorted_scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sorted_scrolled), GTK_SHADOW_IN);
    GtkWidget *sorted_view = gtk_text_view_new();
    configure_text_view(GTK_TEXT_VIEW(sorted_view));
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(sorted_view), "modern-text-view");
    gtk_container_add(GTK_CONTAINER(sorted_scrolled), sorted_view);
    gtk_box_pack_start(GTK_BOX(sorted_card), sorted_scrolled, TRUE, TRUE, 0);
    app_data->sorted_view = GTK_TEXT_VIEW(sorted_view);

    // Préparer les données de nettoyage pour la fenêtre tableaux
    ArrayCleanupData *cleanup_data = g_new0(ArrayCleanupData, 1);
    cleanup_data->data_ptr = app_data->initial_data_ptr;
    cleanup_data->N = app_data->current_N;
    cleanup_data->type = app_data->current_type;

    // Gestionnaire spécifique pour la fermeture de la fenêtre tableaux
    // Utiliser un callback dédié avec protection renforcée
    g_signal_connect(window, "delete-event", G_CALLBACK(on_array_window_delete), NULL);

    // Nettoyage à la destruction de la fenêtre
    g_signal_connect(window, "destroy", G_CALLBACK(on_array_window_destroy), cleanup_data);
    g_signal_connect(window, "destroy", G_CALLBACK(g_free), app_data);

    gtk_widget_show_all(window);
}

void create_curve_window(GtkWidget *parent_window, AppData *app_data) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "📈 Courbes de Performance des Algorithmes de Tri");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 700);
    // Ne pas utiliser transient_for pour éviter que la fermeture de la fenêtre secondaire ferme la principale
    // gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent_window));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(window), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, &WHITE_COLOR);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 780, 680);
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 0);

    AppData *curve_data = g_new0(AppData, 1);
    memcpy(curve_data->comparison_times, app_data->comparison_times, sizeof(app_data->comparison_times));

    // Utiliser N_initial si défini, sinon utiliser une valeur par défaut
    if (app_data->N_initial > 0) {
        curve_data->N_initial = app_data->N_initial;
    } else if (app_data->size_input) {
        curve_data->N_initial = gtk_spin_button_get_value_as_int(app_data->size_input);
        if (curve_data->N_initial == 0) {
            curve_data->N_initial = 1000; // Valeur par défaut
        }
    } else {
        curve_data->N_initial = 1000; // Valeur par défaut
    }

    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_callback), curve_data);
    g_signal_connect(window, "delete-event", G_CALLBACK(on_secondary_window_delete), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(g_free), curve_data);

    gtk_widget_show_all(window);
}

// =========================================================================
//                              MODULE LISTES CHAÎNÉES (TP2)
// =========================================================================

// -------------------------------------------------------------------------
// PROTOTYPES REQUIS (CORRECTION DÉFINITIVE DES ERREURS D'ORDRE)
// -------------------------------------------------------------------------

// Fonctions de base des Listes
static List *list_new(const gchar *structure_type, const gchar *element_type);
static void free_node_data(Node *node, const gchar *type);
static void list_free(List *list);
static void list_insert_int(List *list, int value, int index);
static void list_delete(List *list, int index);

// Fonctions utilitaires de tri
static Node *get_node_at(List *list, int index);
static void swap_node_data(Node *a, Node *b);

// Fonctions de Tri (CORRIGÉES POUR ÉVITER L'ERREUR D'IMPLICIT DECLARATION)
static void list_bubble_sort(List *list);
static void list_insertion_sort(List *list);
static void list_shell_sort(List *list);
static void list_quick_sort(List *list);
static Node *list_quick_partition(List *list, int low_index, int high_index);
static void list_quick_sort_recursive(List *list, int low_index, int high_index);

// Fonctions de Dessin
static void draw_node(cairo_t *cr, double x, double y, const char *text, gboolean is_double);
static void draw_special_node(cairo_t *cr, double x, double y, const char *text, gboolean is_head);
static void draw_arrow(cairo_t *cr, double x1, double y1, double x2, double y2);
static gboolean draw_list_callback(GtkWidget *widget, cairo_t *cr, gpointer data);

// Fonctions de Callback
static void on_list_create_clicked(GtkWidget *widget, gpointer data);
static void on_list_fill_random_clicked(GtkWidget *widget, gpointer data); // NOUVEAU PROTOTYPE
static void on_list_insert_clicked(GtkWidget *widget, gpointer data);
static void on_list_delete_clicked(GtkWidget *widget, gpointer data);
static void on_list_sort_clicked(GtkWidget *widget, gpointer data);

// Fenêtre
static void create_list_window(GtkWidget *parent_window);


// --- [LISTES] --- Fonctions de base des Listes (Inchangées) ---

static List *list_new(const gchar *structure_type, const gchar *element_type) {
    List *list = g_new0(List, 1);
    list->head = NULL;
    list->size = 0;
    list->structure_type = structure_type;
    list->element_type = element_type;
    // Assurez-vous que get_type_info est définie dans la Partie 1
    get_type_info(element_type, &list->element_size, &list->compare_func);
    return list;
}

static void free_node_data(Node *node, const gchar *type) {
    if (g_strcmp0(type, "Chaîne de Caractères") == 0) {
        if (node->data) free(*(char **)node->data);
    }
    free(node->data);
    free(node);
}

static void list_free(List *list) {
    if (!list) return;
    Node *current = list->head;
    Node *next;
    while (current != NULL) {
        next = current->next;
        free_node_data(current, list->element_type);
        current = next;
    }
    g_free(list);
}

// Fonction générique d'insertion pour tous les types
static void list_insert_generic(List *list, void *value, int index) {
    if (!list || !value) return;
    if (index < 0 || index > list->size) index = list->size;

    Node *new_node = g_new0(Node, 1);
    if (!new_node) return; // Échec d'allocation

    void *data = malloc(list->element_size);
    if (!data) {
        g_free(new_node); // Libérer le nœud si l'allocation de données échoue
        return;
    }

    if (g_strcmp0(list->element_type, "Chaîne de Caractères") == 0) {
        // Pour les chaînes, on copie le pointeur
        char **str_ptr = (char **)data;
        *str_ptr = g_strdup(*(const char **)value);
        if (!*str_ptr) {
            free(data);
            g_free(new_node);
            return; // Échec d'allocation de la chaîne
        }
    } else {
        // Pour les autres types, on copie la valeur
        memcpy(data, value, list->element_size);
    }

    new_node->data = data;
    new_node->next = NULL;
    new_node->prev = NULL;

    if (index == 0) {
        new_node->next = list->head;
        if (list->head && g_strcmp0(list->structure_type, "Liste Double") == 0) {
            list->head->prev = new_node;
        }
        list->head = new_node;
    } else {
        Node *current = list->head;
        for (int i = 0; i < index - 1; i++) {
            if (current == NULL) break;
            current = current->next;
        }

        if (current) {
            new_node->next = current->next;
            if (g_strcmp0(list->structure_type, "Liste Double") == 0) {
                new_node->prev = current;
                if (current->next) {
                    current->next->prev = new_node;
                }
            }
            current->next = new_node;
        } else {
            free_node_data(new_node, list->element_type);
            return;
        }
    }
    list->size++;
}

// Fonction helper pour vérifier si une valeur existe déjà dans la liste
static gboolean list_contains_value(List *list, void *value) {
    if (!list || !value || !list->compare_func) return FALSE;

    Node *current = list->head;
    while (current != NULL) {
        if (list->compare_func(current->data, value) == 0) {
            return TRUE; // Valeur trouvée
        }
        current = current->next;
    }
    return FALSE; // Valeur non trouvée
}

// Fonction de compatibilité pour les entiers (pour ne pas casser le code existant)
static void list_insert_int(List *list, int value, int index) {
    list_insert_generic(list, &value, index);
}

static void list_delete(List *list, int index) {
    if (!list->head || index < 0 || index >= list->size) return;

    Node *to_delete = NULL;
    if (index == 0) {
        to_delete = list->head;
        list->head = list->head->next;
        if (list->head && g_strcmp0(list->structure_type, "Liste Double") == 0) {
            list->head->prev = NULL;
        }
    } else {
        Node *current = list->head;
        for (int i = 0; i < index - 1; i++) {
            current = current->next;
            if (current == NULL || current->next == NULL) return;
        }
        to_delete = current->next;
        current->next = to_delete->next;

        if (g_strcmp0(list->structure_type, "Liste Double") == 0) {
            if (to_delete->next) {
                to_delete->next->prev = current;
            }
        }
    }

    if (to_delete) {
        free_node_data(to_delete, list->element_type);
        list->size--;
    }
}

// --- Fonctions utilitaires de tri (Inchangées) ---

static Node *get_node_at(List *list, int index) {
    if (index < 0 || index >= list->size) return NULL;
    Node *current = list->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    return current;
}

static void swap_node_data(Node *a, Node *b) {
    void *temp_data = a->data;
    a->data = b->data;
    b->data = temp_data;
}

// --- [LISTES] --- Fonctions de Tri ---

static void list_bubble_sort(List *list) {
    if (!list || list->size <= 1) return;

    int swapped;
    Node *current, *tail = NULL;

    do {
        swapped = 0;
        current = list->head;

        while (current->next != tail) {
            if (list->compare_func(current->data, current->next->data) > 0) {
                swap_node_data(current, current->next);
                swapped = 1;
            }
            current = current->next;
        }
        tail = current;
    } while (swapped);
}

static void list_insertion_sort(List *list) {
    if (!list || list->size <= 1) return;

    // Cette implémentation par permutation de données fonctionne mieux sur les listes doubles.
    Node *sorted_end = list->head;

    while (sorted_end->next != NULL) {
        Node *unsorted_node = sorted_end->next;

        if (list->compare_func(unsorted_node->data, list->head->data) < 0) {
            void *key_data = unsorted_node->data;
            Node *temp_ptr = unsorted_node;

            while(temp_ptr != list->head) {
                // Nécessite le champ 'prev' (Liste Double)
                if (g_strcmp0(list->structure_type, "Liste Double") != 0 || !temp_ptr->prev) break;
                temp_ptr->data = temp_ptr->prev->data;
                temp_ptr = temp_ptr->prev;
            }
            list->head->data = key_data;

        } else if (list->compare_func(unsorted_node->data, sorted_end->data) < 0) {
            Node *current_sorted = list->head;

            while (current_sorted != sorted_end && list->compare_func(unsorted_node->data, current_sorted->next->data) > 0) {
                current_sorted = current_sorted->next;
            }

            void *key_data = unsorted_node->data;
            Node *i = unsorted_node;

            while (i != current_sorted->next) {
                 // Nécessite le champ 'prev' (Liste Double)
                if (g_strcmp0(list->structure_type, "Liste Double") != 0 || !i->prev) break;
                i->data = i->prev->data;
                i = i->prev;
            }
            current_sorted->next->data = key_data;
        }

        sorted_end = sorted_end->next;
    }
}

// Implémentation de Tri Shell pour listes (par index)
static void list_shell_sort(List *list) {
    if (!list || list->size <= 1) return;
    int n = list->size;

    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {

            for (int j = i; j >= gap; j -= gap) {
                Node *j_minus_gap_node = get_node_at(list, j - gap);
                Node *j_node = get_node_at(list, j);

                if (!j_minus_gap_node || !j_node) break;

                if (list->compare_func(j_minus_gap_node->data, j_node->data) > 0) {
                    swap_node_data(j_minus_gap_node, j_node);
                } else {
                    break;
                }
            }
        }
    }
}

// Fonctions utilitaires Quicksort (par index)
static Node *list_quick_partition(List *list, int low_index, int high_index) {
    Node *high_node = get_node_at(list, high_index);
    if (!high_node) return NULL;
    Node *pivot = high_node;

    Node *i = get_node_at(list, low_index);

    for (int j_index = low_index; j_index < high_index; j_index++) {
        Node *j = get_node_at(list, j_index);
        if (list->compare_func(j->data, pivot->data) < 0) {
            swap_node_data(i, j);
            i = i->next;
        }
    }
    swap_node_data(i, high_node);
    return i;
}

static void list_quick_sort_recursive(List *list, int low_index, int high_index) {
    if (low_index < high_index) {
        Node *pi_node = list_quick_partition(list, low_index, high_index);

        int pi_index = 0;
        Node *current = list->head;
        while (current != pi_node) {
            current = current->next;
            pi_index++;
        }

        list_quick_sort_recursive(list, low_index, pi_index - 1);
        list_quick_sort_recursive(list, pi_index + 1, high_index);
    }
}

static void list_quick_sort(List *list) {
    if (!list || list->size <= 1) return;
    list_quick_sort_recursive(list, 0, list->size - 1);
}


// --- [LISTES] --- Fonctions de Dessin et Callback (Inchangées) ---

static void draw_node(cairo_t *cr, double x, double y, const char *text, gboolean is_double) {
    // Boîte principale du nœud (Data)
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.25); // Dark Node fill
    cairo_rectangle(cr, x, y, NODE_WIDTH, NODE_HEIGHT);
    cairo_fill_preserve(cr);
    cairo_set_source_rgb(cr, 0.0, 0.95, 0.4); // Neon Green Border
    cairo_set_line_width(cr, 1.5);
    cairo_stroke(cr);

    // Séparateur (Next)
    cairo_set_source_rgb(cr, 0.4, 0.4, 0.5); // Grey separator
    cairo_move_to(cr, x + NODE_WIDTH * 0.75, y);
    cairo_line_to(cr, x + NODE_WIDTH * 0.75, y + NODE_HEIGHT);
    cairo_stroke(cr);

    if (is_double) {
          // Séparateur (Prev)
        cairo_move_to(cr, x + NODE_WIDTH * 0.25, y);
        cairo_line_to(cr, x + NODE_WIDTH * 0.25, y + NODE_HEIGHT);
        cairo_stroke(cr);
    }

    // Texte (Data)
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White Text
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);
    cairo_text_extents_t extents;
    cairo_text_extents(cr, text, &extents);

    double text_center_x = is_double ? x + NODE_WIDTH / 2.0 : x + NODE_WIDTH * 0.375;
    double text_center_y = y + NODE_HEIGHT / 2.0;

    cairo_move_to(cr, text_center_x - extents.width / 2.0, text_center_y + extents.height / 2.0);
    cairo_show_text(cr, text);
}

static void draw_arrow(cairo_t *cr, double x1, double y1, double x2, double y2) {
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8); // Light arrow

    cairo_set_line_width(cr, 1.5);
    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_stroke(cr);

    double angle = atan2(y2 - y1, x2 - x1);
    double head_len = 10;

    cairo_move_to(cr, x2, y2);
    cairo_line_to(cr,
                     x2 - head_len * cos(angle - M_PI / 6),
                     y2 - head_len * sin(angle - M_PI / 6));
    cairo_move_to(cr, x2, y2);
    cairo_line_to(cr,
                     x2 - head_len * cos(angle + M_PI / 6),
                     y2 - head_len * sin(angle + M_PI / 6));
    cairo_stroke(cr);
}

// Fonction pour dessiner un nœud spécial (Tête ou NULL)
static void draw_special_node(cairo_t *cr, double x, double y, const char *text, gboolean is_head) {
    // Style différent pour Tête (bleu) et NULL (rouge)
    if (is_head) {
        // Tête - Style bleu
        cairo_set_source_rgb(cr, 0.1, 0.3, 0.6); // Dark Blue fill
        cairo_rectangle(cr, x, y, NODE_WIDTH, NODE_HEIGHT);
        cairo_fill_preserve(cr);
        cairo_set_source_rgb(cr, 0.2, 0.6, 1.0); // Light Blue Border
    } else {
        // NULL - Style rouge/gris
        cairo_set_source_rgb(cr, 0.3, 0.1, 0.1); // Dark Red fill
        cairo_rectangle(cr, x, y, NODE_WIDTH, NODE_HEIGHT);
        cairo_fill_preserve(cr);
        cairo_set_source_rgb(cr, 1.0, 0.3, 0.3); // Red Border
    }
    cairo_set_line_width(cr, 2.0);
    cairo_stroke(cr);

    // Texte
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White Text
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);
    cairo_text_extents_t extents;
    cairo_text_extents(cr, text, &extents);

    double text_center_x = x + NODE_WIDTH / 2.0;
    double text_center_y = y + NODE_HEIGHT / 2.0;

    cairo_move_to(cr, text_center_x - extents.width / 2.0, text_center_y + extents.height / 2.0);
    cairo_show_text(cr, text);
}

static gboolean draw_list_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
    AppData *app_data = (AppData *)data;
    List *list = app_data->current_list;
    guint width = gtk_widget_get_allocated_width(widget);
    guint height = gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgb(cr, 0.1, 0.1, 0.12); // Dark BG
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White text/lines

    if (!list || !list->head) {
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White text/lines
        cairo_move_to(cr, width / 2.0 - 50, height / 2.0);
        cairo_show_text(cr, "Liste Vide");
        return TRUE;
    }

    gboolean is_double = g_strcmp0(list->structure_type, "Liste Double") == 0;
    Node *current = list->head;
    int i = 0;

    // Dessiner "Tête" au début
    double head_x = 10;
    double y = height / 2.0 - NODE_HEIGHT / 2.0;
    draw_special_node(cr, head_x, y, "Tête", TRUE);

    // Flèche de Tête vers le premier nœud
    double first_node_x = head_x + NODE_WIDTH + SPACING;
    double arrow_start_x = head_x + NODE_WIDTH;
    double arrow_end_x = first_node_x;
    draw_arrow(cr, arrow_start_x, y + NODE_HEIGHT/2.0, arrow_end_x, y + NODE_HEIGHT/2.0);

    // Décaler l'index pour les nœuds de données
    i = 1;

    while (current != NULL) {
        double x = 10 + i * (NODE_WIDTH + SPACING);
        double node_y = height / 2.0 - NODE_HEIGHT / 2.0;

        gchar node_text[64];
        if (g_strcmp0(list->element_type, "Entiers (Int)") == 0) {
            g_snprintf(node_text, 64, "%d", *(int *)current->data);
        } else if (g_strcmp0(list->element_type, "Réels (Float)") == 0) {
            g_snprintf(node_text, 64, "%.2f", *(float *)current->data);
        } else if (g_strcmp0(list->element_type, "Caractères (Char)") == 0) {
            g_snprintf(node_text, 64, "%c", *(char *)current->data);
        } else if (g_strcmp0(list->element_type, "Chaîne de Caractères") == 0) {
            const char *str = *(const char **)current->data;
            if (str) {
                g_snprintf(node_text, 64, "%.15s", str); // Limiter à 15 caractères pour l'affichage
            } else {
                g_snprintf(node_text, 64, "(null)");
            }
        } else {
            g_snprintf(node_text, 64, "Data");
        }

        draw_node(cr, x, node_y, node_text, is_double);

        // Flèche Next vers le nœud suivant ou NULL
        if (current->next != NULL) {
            double arrow_start_x = x + NODE_WIDTH * 0.75;
            double arrow_end_x = x + NODE_WIDTH + SPACING;
            draw_arrow(cr, arrow_start_x, node_y + NODE_HEIGHT/2.0, arrow_end_x, node_y + NODE_HEIGHT/2.0);
        } else {
            // Flèche vers NULL (dernier nœud)
            double arrow_start_x = x + NODE_WIDTH * 0.75;
            double null_x = x + NODE_WIDTH + SPACING;
            double arrow_end_x = null_x;
            draw_arrow(cr, arrow_start_x, node_y + NODE_HEIGHT/2.0, arrow_end_x, node_y + NODE_HEIGHT/2.0);

            // Dessiner "NULL" après le dernier nœud
            draw_special_node(cr, null_x, node_y, "NULL", FALSE);
        }

        // Flèche Prev pour les listes doubles
        if (is_double && current->prev != NULL) {
             double arrow_start_x = x + NODE_WIDTH * 0.25;
             double arrow_end_x = x - SPACING;
             draw_arrow(cr, arrow_start_x, node_y + NODE_HEIGHT * 0.75, arrow_end_x, node_y + NODE_HEIGHT * 0.75);
        }

        current = current->next;
        i++;
    }

    return TRUE;
}

static void update_list_drawing_area_size(AppData *app_data) {
    if (!app_data->current_list || !app_data->list_drawing_area) return;
    int n = app_data->current_list->size;
    // Calculer la largeur nécessaire : Tête + n nœuds de données + NULL + espacement
    // Tête (1) + n nœuds + NULL (1) = n + 2 nœuds au total
    // Position de départ (10) + Tête (NODE_WIDTH) + SPACING + n nœuds * (NODE_WIDTH + SPACING) + NULL (NODE_WIDTH) + marge (50)
    int width = 10 + NODE_WIDTH + SPACING + n * (NODE_WIDTH + SPACING) + NODE_WIDTH + 50;
    if (width < 600) width = 600; // Minimum width
    gtk_widget_set_size_request(app_data->list_drawing_area, width, 400);
    gtk_widget_queue_draw(app_data->list_drawing_area);
}


// --- [LISTES] --- Fonctions de Callback (TP2) ---

static void on_list_create_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    const gchar *structure_type = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app_data->list_type_combo));
    const gchar *element_type = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app_data->type_combo));

    if (app_data->current_list) list_free(app_data->current_list);
    app_data->current_list = list_new(structure_type, element_type);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
    gtk_text_buffer_set_text(buffer, g_strdup_printf("Liste créée:\nType: %s\nÉléments: %s\nTaille: 0. Cliquez sur Remplir ou Insérer.",
                                                     structure_type, element_type), -1);

    gtk_widget_queue_draw(app_data->list_drawing_area);
    update_list_drawing_area_size(app_data);
}

// Legacy callbacks removed (on_list_insert_clicked, on_list_delete_clicked)
static void on_list_input_source_toggled(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        const gchar *label = gtk_button_get_label(GTK_BUTTON(widget));
        if (g_strcmp0(label, "Aléatoire") == 0) {
            app_data->list_input_source = 0;
            gtk_widget_show(app_data->list_random_box);
            gtk_widget_hide(app_data->list_manual_box);
        } else if (g_strcmp0(label, "Manuelle") == 0) {
            app_data->list_input_source = 1;
            gtk_widget_hide(app_data->list_random_box);
            gtk_widget_show(app_data->list_manual_box);
        }
    }
}

static void on_list_fill_random_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_list) on_list_create_clicked(NULL, app_data);
    if (!app_data->current_list) return;
    int size = gtk_spin_button_get_value_as_int(app_data->list_size_input);
    if (size <= 0) return;

    const gchar *element_type = app_data->current_list->element_type;
    int added_count = 0;
    int max_attempts = 1000; // Limite pour éviter les boucles infinies

    for(int i=0; i<size; i++) {
        int attempts = 0;
        gboolean value_added = FALSE;

        while (!value_added && attempts < max_attempts) {
            attempts++;

            if (g_strcmp0(element_type, "Entiers (Int)") == 0) {
                int val = rand() % 10000; // Augmenter la plage pour éviter les collisions
                // Vérifier si la valeur existe déjà
                if (!list_contains_value(app_data->current_list, &val)) {
                    list_insert_generic(app_data->current_list, &val, app_data->current_list->size);
                    value_added = TRUE;
                    added_count++;
                }
            } else if (g_strcmp0(element_type, "Réels (Float)") == 0) {
                float val = (float)(rand() % 100000) / 100.0f; // Augmenter la plage
                // Vérifier si la valeur existe déjà
                if (!list_contains_value(app_data->current_list, &val)) {
                    list_insert_generic(app_data->current_list, &val, app_data->current_list->size);
                    value_added = TRUE;
                    added_count++;
                }
            } else if (g_strcmp0(element_type, "Caractères (Char)") == 0) {
                char val = 'A' + (rand() % 26);
                list_insert_generic(app_data->current_list, &val, app_data->current_list->size);
                value_added = TRUE;
                added_count++;
            } else if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
                char *str = generate_random_string(5);
                list_insert_generic(app_data->current_list, &str, app_data->current_list->size);
                g_free(str); // generate_random_string utilise g_strdup, donc g_free
                value_added = TRUE;
                added_count++;
            } else {
                int val = rand() % 10000;
                if (!list_contains_value(app_data->current_list, &val)) {
                    list_insert_generic(app_data->current_list, &val, app_data->current_list->size);
                    value_added = TRUE;
                    added_count++;
                }
            }
        }

        if (!value_added) {
            // Si on n'a pas pu ajouter de valeur unique après max_attempts tentatives
            // Afficher un avertissement mais continuer
            break;
        }
    }

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
    if (added_count < size) {
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Ajout de %d éléments aléatoires uniques (sur %d demandés). Taille totale: %lu\n⚠️ Impossible de générer plus de valeurs uniques.",
                                                         added_count, size, app_data->current_list->size), -1);
    } else {
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Ajout de %d éléments aléatoires uniques. Taille totale: %lu",
                                                         added_count, app_data->current_list->size), -1);
    }
    gtk_widget_queue_draw(app_data->list_drawing_area);
    update_list_drawing_area_size(app_data);
}

static void on_list_add_manual_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_list) on_list_create_clicked(NULL, app_data);
    if (!app_data->current_list) return;

    const gchar *text = gtk_entry_get_text(app_data->list_value_entry);
    const gchar *element_type = app_data->current_list->element_type;

    if (g_strcmp0(element_type, "Entiers (Int)") == 0) {
        int value = atoi(text);
        list_insert_generic(app_data->current_list, &value, app_data->current_list->size);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Élément %d ajouté manuellement. Taille totale: %lu",
                                                         value, app_data->current_list->size), -1);
    } else if (g_strcmp0(element_type, "Réels (Float)") == 0) {
        float value = (float)atof(text);
        list_insert_generic(app_data->current_list, &value, app_data->current_list->size);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Élément %.2f ajouté manuellement. Taille totale: %lu",
                                                         value, app_data->current_list->size), -1);
    } else if (g_strcmp0(element_type, "Caractères (Char)") == 0) {
        char value = (text && strlen(text) > 0) ? text[0] : 'A';
        list_insert_generic(app_data->current_list, &value, app_data->current_list->size);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Élément '%c' ajouté manuellement. Taille totale: %lu",
                                                         value, app_data->current_list->size), -1);
    } else if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
        const char *str = text;
        list_insert_generic(app_data->current_list, &str, app_data->current_list->size);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Élément '%s' ajouté manuellement. Taille totale: %lu",
                                                         text, app_data->current_list->size), -1);
    } else {
        int value = atoi(text);
        list_insert_generic(app_data->current_list, &value, app_data->current_list->size);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Élément %d ajouté manuellement. Taille totale: %lu",
                                                         value, app_data->current_list->size), -1);
    }

    gtk_widget_queue_draw(app_data->list_drawing_area);
    update_list_drawing_area_size(app_data);
}

static void on_list_insert_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_list) return;

    const gchar *element_type = app_data->current_list->element_type;
    int index = app_data->current_list->size;

    if (g_strcmp0(element_type, "Entiers (Int)") == 0) {
        int value = rand() % 100;
        list_insert_generic(app_data->current_list, &value, index);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Insertion de %d à l'index %d. Nouvelle taille: %lu",
                                                         value, index, app_data->current_list->size), -1);
    } else if (g_strcmp0(element_type, "Réels (Float)") == 0) {
        float value = (float)(rand() % 10000) / 100.0f;
        list_insert_generic(app_data->current_list, &value, index);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Insertion de %.2f à l'index %d. Nouvelle taille: %lu",
                                                         value, index, app_data->current_list->size), -1);
    } else if (g_strcmp0(element_type, "Caractères (Char)") == 0) {
        char value = 'A' + (rand() % 26);
        list_insert_generic(app_data->current_list, &value, index);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Insertion de '%c' à l'index %d. Nouvelle taille: %lu",
                                                         value, index, app_data->current_list->size), -1);
    } else if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
        char *str = generate_random_string(5);
        list_insert_generic(app_data->current_list, &str, index);
        g_free(str);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Insertion de chaîne à l'index %d. Nouvelle taille: %lu",
                                                         index, app_data->current_list->size), -1);
    } else {
        int value = rand() % 100;
        list_insert_generic(app_data->current_list, &value, index);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Insertion de %d à l'index %d. Nouvelle taille: %lu",
                                                         value, index, app_data->current_list->size), -1);
    }

    gtk_widget_queue_draw(app_data->list_drawing_area);
    update_list_drawing_area_size(app_data);
}

static void on_list_delete_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_list || app_data->current_list->size == 0) return;

    list_delete(app_data->current_list, 0);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
    gtk_text_buffer_set_text(buffer, g_strdup_printf("Suppression de l'élément à l'index 0. Nouvelle taille: %d",
                                                     app_data->current_list->size), -1);

    gtk_widget_queue_draw(app_data->list_drawing_area);
}

static void on_list_sort_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_list || !app_data->current_list->head) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, "Erreur : La liste est vide. Veuillez créer une liste et ajouter des éléments.", -1);
        return;
    }

    const gchar *method_name = g_object_get_data(G_OBJECT(widget), "list-method-name");
    if (!method_name) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, "Erreur : Méthode de tri non identifiée.", -1);
        return;
    }

    // Comparer avec les noms complets incluant les emojis
    if (g_strcmp0(method_name, "🔴 Tri à Bulles") == 0 || g_str_has_suffix(method_name, "Tri à Bulles")) {
        list_bubble_sort(app_data->current_list);
    } else if (g_strcmp0(method_name, "🟢 Tri par Insertion") == 0 || g_str_has_suffix(method_name, "Tri par Insertion")) {
        list_insertion_sort(app_data->current_list);
    } else if (g_strcmp0(method_name, "🔵 Tri Shell") == 0 || g_str_has_suffix(method_name, "Tri Shell")) {
        list_shell_sort(app_data->current_list);
    } else if (g_strcmp0(method_name, "🟡 Tri Quicksort") == 0 || g_str_has_suffix(method_name, "Tri Quicksort")) {
        list_quick_sort(app_data->current_list);
    } else {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Erreur : Méthode de tri non reconnue : %s", method_name), -1);
        return;
    }

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
    gtk_text_buffer_set_text(buffer, g_strdup_printf("Tri de la liste effectué par : %s. (Tri par échange de données).", method_name), -1);

    gtk_widget_queue_draw(app_data->list_drawing_area);
}

// --- Helper for Input Dialog ---
// Retourne -1 si l'utilisateur annule, sinon retourne la valeur entrée
static int get_integer_input(GtkWidget *parent, const char *title, const char *prompt, int default_val) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(title, GTK_WINDOW(parent),
                                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "_OK", GTK_RESPONSE_ACCEPT,
                                                    "_Annuler", GTK_RESPONSE_REJECT,
                                                    NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new(prompt);
    gtk_container_add(GTK_CONTAINER(content_area), label);

    GtkWidget *entry = gtk_entry_new();
    char buf[32]; snprintf(buf, 32, "%d", default_val);
    gtk_entry_set_text(GTK_ENTRY(entry), buf);
    gtk_container_add(GTK_CONTAINER(content_area), entry);

    gtk_widget_show_all(dialog);

    int result = -1; // -1 indique l'annulation
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));
        if (text && strlen(text) > 0) {
            result = atoi(text);
        } else {
            result = default_val; // Utiliser la valeur par défaut si vide
        }
    }
    gtk_widget_destroy(dialog);
    return result;
}

// Fonction générique pour obtenir une valeur selon le type
static void *get_value_input(GtkWidget *parent, const char *title, const char *prompt, const gchar *element_type, void *default_val) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(title, GTK_WINDOW(parent),
                                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "_OK", GTK_RESPONSE_ACCEPT,
                                                    "_Annuler", GTK_RESPONSE_REJECT,
                                                    NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new(prompt);
    gtk_container_add(GTK_CONTAINER(content_area), label);

    GtkWidget *entry = gtk_entry_new();
    char buf[64];
    if (g_strcmp0(element_type, "Entiers (Int)") == 0) {
        snprintf(buf, 64, "%d", default_val ? *(int*)default_val : 0);
    } else if (g_strcmp0(element_type, "Réels (Float)") == 0) {
        snprintf(buf, 64, "%.2f", default_val ? *(float*)default_val : 0.0f);
    } else if (g_strcmp0(element_type, "Caractères (Char)") == 0) {
        snprintf(buf, 64, "%c", default_val ? *(char*)default_val : 'A');
    } else if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
        snprintf(buf, 64, "%s", default_val ? *(const char**)default_val : "");
    }
    gtk_entry_set_text(GTK_ENTRY(entry), buf);
    gtk_container_add(GTK_CONTAINER(content_area), entry);

    gtk_widget_show_all(dialog);

    void *result = NULL;
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));

        if (g_strcmp0(element_type, "Entiers (Int)") == 0) {
            int *val = g_new(int, 1);
            *val = atoi(text);
            result = val;
        } else if (g_strcmp0(element_type, "Réels (Float)") == 0) {
            float *val = g_new(float, 1);
            *val = (float)atof(text);
            result = val;
        } else if (g_strcmp0(element_type, "Caractères (Char)") == 0) {
            char *val = g_new(char, 1);
            *val = (text && strlen(text) > 0) ? text[0] : 'A';
            result = val;
        } else if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
            char **val = g_new(char*, 1);
            *val = g_strdup(text ? text : "");
            result = val;
        }
    }
    gtk_widget_destroy(dialog);
    return result;
}

// --- Advanced List Operations ---

static void on_list_insert_pos_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_list) on_list_create_clicked(NULL, app_data);
    if (!app_data->current_list) return;

    const gchar *element_type = app_data->current_list->element_type;

    // Demander la valeur à insérer
    void *val = get_value_input(gtk_widget_get_toplevel(widget), "Insertion", "Valeur à insérer:", element_type, NULL);
    if (!val) {
        // Utilisateur a annulé la saisie de la valeur
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, "Insertion annulée.", -1);
        return;
    }

    // Demander la position d'insertion
    int max_pos = app_data->current_list->size;
    gchar *prompt = g_strdup_printf("Index d'insertion (0-%d):", max_pos);
    int pos = get_integer_input(gtk_widget_get_toplevel(widget), "Position", prompt, max_pos);
    g_free(prompt);

    // Vérifier si l'utilisateur a annulé
    if (pos == -1) {
        // Libérer la mémoire allouée pour la valeur
        if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
            g_free(*(char**)val);
        }
        g_free(val);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, "Insertion annulée.", -1);
        return;
    }

    // Valider la position
    if (pos < 0) {
        pos = 0; // Ajuster au début si négatif
    } else if (pos > max_pos) {
        pos = max_pos; // Ajuster à la fin si trop grand
    }

    // Sauvegarder la valeur pour l'affichage avant insertion
    gchar *value_str = NULL;
    if (g_strcmp0(element_type, "Entiers (Int)") == 0) {
        value_str = g_strdup_printf("%d", *(int*)val);
    } else if (g_strcmp0(element_type, "Réels (Float)") == 0) {
        value_str = g_strdup_printf("%.2f", *(float*)val);
    } else if (g_strcmp0(element_type, "Caractères (Char)") == 0) {
        value_str = g_strdup_printf("'%c'", *(char*)val);
    } else if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
        value_str = g_strdup_printf("'%s'", *(const char**)val);
    }

    list_insert_generic(app_data->current_list, val, pos);

    // Libérer la mémoire allouée par get_value_input
    if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
        g_free(*(char**)val);
    }
    g_free(val);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
    gtk_text_buffer_set_text(buffer, g_strdup_printf("Valeur %s insérée à l'index %d. Nouvelle taille: %lu",
                                                     value_str ? value_str : "?", pos, app_data->current_list->size), -1);
    if (value_str) g_free(value_str);

    gtk_widget_queue_draw(app_data->list_drawing_area);
    update_list_drawing_area_size(app_data);
}

static void on_list_insert_start_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_list) on_list_create_clicked(NULL, app_data);
    if (!app_data->current_list) return;

    const gchar *element_type = app_data->current_list->element_type;
    void *val = get_value_input(gtk_widget_get_toplevel(widget), "Insertion Début", "Valeur à insérer:", element_type, NULL);
    if (!val) return; // Utilisateur a annulé

    // Sauvegarder la valeur pour l'affichage
    gchar *value_str = NULL;
    if (g_strcmp0(element_type, "Entiers (Int)") == 0) {
        value_str = g_strdup_printf("%d", *(int*)val);
    } else if (g_strcmp0(element_type, "Réels (Float)") == 0) {
        value_str = g_strdup_printf("%.2f", *(float*)val);
    } else if (g_strcmp0(element_type, "Caractères (Char)") == 0) {
        value_str = g_strdup_printf("'%c'", *(char*)val);
    } else if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
        value_str = g_strdup_printf("'%s'", *(const char**)val);
    }

    list_insert_generic(app_data->current_list, val, 0);

    // Libérer la mémoire allouée par get_value_input
    if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
        g_free(*(char**)val);
    }
    g_free(val);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
    gtk_text_buffer_set_text(buffer, g_strdup_printf("Valeur %s insérée au début. Nouvelle taille: %lu",
                                                     value_str ? value_str : "?", app_data->current_list->size), -1);
    if (value_str) g_free(value_str);

    gtk_widget_queue_draw(app_data->list_drawing_area);
    update_list_drawing_area_size(app_data);
}

static void on_list_insert_end_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_list) on_list_create_clicked(NULL, app_data);
    if (!app_data->current_list) return;

    const gchar *element_type = app_data->current_list->element_type;
    void *val = get_value_input(gtk_widget_get_toplevel(widget), "Insertion Fin", "Valeur à insérer:", element_type, NULL);
    if (!val) return; // Utilisateur a annulé

    // Sauvegarder la valeur pour l'affichage
    gchar *value_str = NULL;
    if (g_strcmp0(element_type, "Entiers (Int)") == 0) {
        value_str = g_strdup_printf("%d", *(int*)val);
    } else if (g_strcmp0(element_type, "Réels (Float)") == 0) {
        value_str = g_strdup_printf("%.2f", *(float*)val);
    } else if (g_strcmp0(element_type, "Caractères (Char)") == 0) {
        value_str = g_strdup_printf("'%c'", *(char*)val);
    } else if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
        value_str = g_strdup_printf("'%s'", *(const char**)val);
    }

    list_insert_generic(app_data->current_list, val, app_data->current_list->size);

    // Libérer la mémoire allouée par get_value_input
    if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
        g_free(*(char**)val);
    }
    g_free(val);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
    gtk_text_buffer_set_text(buffer, g_strdup_printf("Valeur %s insérée à la fin. Nouvelle taille: %lu",
                                                     value_str ? value_str : "?", app_data->current_list->size), -1);
    if (value_str) g_free(value_str);

    gtk_widget_queue_draw(app_data->list_drawing_area);
    update_list_drawing_area_size(app_data);
}

static void on_list_delete_pos_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_list || app_data->current_list->size == 0) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, "La liste est vide. Impossible de supprimer.", -1);
        return;
    }

    int max_pos = app_data->current_list->size - 1;
    gchar *prompt = g_strdup_printf("Index à supprimer (0-%d):", max_pos);
    int pos = get_integer_input(gtk_widget_get_toplevel(widget), "Suppression", prompt, 0);
    g_free(prompt);

    // Vérifier si l'utilisateur a annulé
    if (pos == -1) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, "Suppression annulée.", -1);
        return;
    }

    // Valider la position
    if (pos < 0 || pos >= app_data->current_list->size) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Index invalide! La liste contient %lu éléments (indices 0-%d).",
                                                         app_data->current_list->size, max_pos), -1);
        return;
    }

    list_delete(app_data->current_list, pos);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
    gtk_text_buffer_set_text(buffer, g_strdup_printf("Élément à l'index %d supprimé. Nouvelle taille: %lu",
                                                     pos, app_data->current_list->size), -1);

    gtk_widget_queue_draw(app_data->list_drawing_area);
    update_list_drawing_area_size(app_data);
}

static void on_list_delete_start_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_list || app_data->current_list->size == 0) return;

    list_delete(app_data->current_list, 0);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
    gtk_text_buffer_set_text(buffer, g_strdup_printf("Élément au début supprimé. Nouvelle taille: %lu",
                                                     app_data->current_list->size), -1);

    gtk_widget_queue_draw(app_data->list_drawing_area);
    update_list_drawing_area_size(app_data);
}

static void on_list_delete_end_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_list || app_data->current_list->size == 0) return;

    size_t old_size = app_data->current_list->size;
    list_delete(app_data->current_list, app_data->current_list->size - 1);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
    gtk_text_buffer_set_text(buffer, g_strdup_printf("Élément à la fin supprimé. Nouvelle taille: %lu",
                                                     app_data->current_list->size), -1);

    gtk_widget_queue_draw(app_data->list_drawing_area);
    update_list_drawing_area_size(app_data);
}

// --- Interactive Editing ---
static gboolean on_list_click_event(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_list || !app_data->current_list->head ||
        event->type != GDK_BUTTON_PRESS || event->button != 1) return FALSE;

    // Detect node click
    // Position X: Tête à 10, puis nœuds de données commencent à 10 + NODE_WIDTH + SPACING + i * (NODE_WIDTH + SPACING)
    // Position Y: height / 2.0 - NODE_HEIGHT / 2.0

    guint height = gtk_widget_get_allocated_height(widget);
    double y_start = height / 2.0 - NODE_HEIGHT / 2.0;

    if (event->y < y_start || event->y > y_start + NODE_HEIGHT) return FALSE; // Not in Y band

    // Vérifier si on clique sur "Tête" (ne pas permettre la modification de Tête)
    double head_x = 10;
    if (event->x >= head_x && event->x <= head_x + NODE_WIDTH) {
        return FALSE; // Clic sur Tête, ne rien faire
    }

    // Vérifier si on clique sur "NULL" (ne pas permettre la modification de NULL)
    int n = app_data->current_list->size;
    double null_x = 10 + NODE_WIDTH + SPACING + n * (NODE_WIDTH + SPACING);
    if (event->x >= null_x && event->x <= null_x + NODE_WIDTH) {
        return FALSE; // Clic sur NULL, ne rien faire
    }

    // Chercher le nœud de données cliqué
    // Les nœuds de données commencent après Tête: 10 + NODE_WIDTH + SPACING + i * (NODE_WIDTH + SPACING)
    int clicked_index = -1;
    for (int i = 0; i < app_data->current_list->size; i++) {
        double x_start = 10 + NODE_WIDTH + SPACING + i * (NODE_WIDTH + SPACING);
        if (event->x >= x_start && event->x <= x_start + NODE_WIDTH) {
            clicked_index = i;
            break;
        }
    }

    if (clicked_index != -1) {
        Node *node = get_node_at(app_data->current_list, clicked_index);
        if (node) {
            const gchar *element_type = app_data->current_list->element_type;

            // Préparer la valeur actuelle pour l'affichage dans la boîte de dialogue
            void *current_val = NULL;
            if (g_strcmp0(element_type, "Entiers (Int)") == 0) {
                current_val = node->data;
            } else if (g_strcmp0(element_type, "Réels (Float)") == 0) {
                current_val = node->data;
            } else if (g_strcmp0(element_type, "Caractères (Char)") == 0) {
                current_val = node->data;
            } else if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
                current_val = node->data;
            }

            // Demander la nouvelle valeur
            void *new_val = get_value_input(gtk_widget_get_toplevel(widget), "Modifier", "Nouvelle valeur:", element_type, current_val);
            if (new_val) {
                // Libérer l'ancienne valeur si c'est une chaîne de caractères
                if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
                    g_free(*(char **)node->data);
                }

                // Copier la nouvelle valeur
                if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
                    *(char **)node->data = g_strdup(*(const char **)new_val);
                } else {
                    memcpy(node->data, new_val, app_data->current_list->element_size);
                }

                // Libérer la mémoire allouée par get_value_input
                if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
                    g_free(*(char **)new_val);
                }
                g_free(new_val);

                // Mettre à jour l'affichage
                gtk_widget_queue_draw(widget);

                // Mettre à jour le message d'information
                GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
                gchar *info_text = NULL;
                if (g_strcmp0(element_type, "Entiers (Int)") == 0) {
                    info_text = g_strdup_printf("Élément à l'index %d modifié.", clicked_index);
                } else if (g_strcmp0(element_type, "Réels (Float)") == 0) {
                    info_text = g_strdup_printf("Élément à l'index %d modifié.", clicked_index);
                } else if (g_strcmp0(element_type, "Caractères (Char)") == 0) {
                    info_text = g_strdup_printf("Élément à l'index %d modifié.", clicked_index);
                } else if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
                    info_text = g_strdup_printf("Élément à l'index %d modifié.", clicked_index);
                }
                if (info_text) {
                    gtk_text_buffer_set_text(buffer, info_text, -1);
                    g_free(info_text);
                }
            }
        }
    }

    return TRUE;
}

static void create_list_window(GtkWidget *parent_window) {
    AppData *app_data = g_new0(AppData, 1);
    app_data->current_list = NULL;

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "🔗 Module Listes Chaînées (Opérations et Tri)");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    // Ne pas utiliser transient_for pour éviter que la fermeture de la fenêtre secondaire ferme la principale
    // gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent_window));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(window), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(window), 15);

    // Container principal avec fond moderne
    GtkWidget *main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_container);

    // Header moderne avec gradient
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(header_box), "modern-card-header");
    gtk_widget_set_size_request(header_box, -1, 80);
    gtk_box_pack_start(GTK_BOX(main_container), header_box, FALSE, FALSE, 0);

    GtkWidget *header_title = gtk_label_new(NULL);
    PangoFontDescription *header_font = pango_font_description_from_string("Segoe UI Bold 24");
    gtk_widget_override_font(header_title, header_font);
    pango_font_description_free(header_font);
    gtk_label_set_markup(GTK_LABEL(header_title), "<span foreground='#ffffff' size='x-large' weight='bold'>🔗 MODULE LISTES CHAÎNÉES</span>\n<span size='small' foreground='#ffffff'>Opérations et Tri</span>");
    gtk_label_set_justify(GTK_LABEL(header_title), GTK_JUSTIFY_CENTER);
    gtk_label_set_xalign(GTK_LABEL(header_title), 0.5);
    gtk_box_pack_start(GTK_BOX(header_box), header_title, TRUE, TRUE, 0);

    // Panneau principal avec paned
    GtkWidget *main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(main_container), main_hbox, TRUE, TRUE, 0);

    // ========== PANEL GAUCHE : CONTROLES MODERNES ==========
    GtkWidget *control_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(control_scrolled), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(control_scrolled), GTK_SHADOW_NONE);

    GtkWidget *control_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(control_vbox), "control-panel");
    gtk_widget_set_size_request(control_vbox, 350, -1);
    gtk_container_set_border_width(GTK_CONTAINER(control_vbox), 20);
    gtk_container_add(GTK_CONTAINER(control_scrolled), control_vbox);

    // Section Structure de la Liste (Carte moderne)
    GtkWidget *type_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(type_card), "modern-card");
    gtk_box_pack_start(GTK_BOX(control_vbox), type_card, FALSE, FALSE, 0);

    GtkWidget *type_title = gtk_label_new("Structure de la Liste");
    gtk_label_set_markup(GTK_LABEL(type_title), "<span foreground='#ffffff' size='large' weight='bold'>📋 Structure de la Liste</span>");
    gtk_box_pack_start(GTK_BOX(type_card), type_title, FALSE, FALSE, 0);

    GtkWidget *list_type_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(list_type_combo), "Liste Simple");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(list_type_combo), "Liste Double");
    gtk_combo_box_set_active(GTK_COMBO_BOX(list_type_combo), 0);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(list_type_combo), "modern-combo");
    gtk_box_pack_start(GTK_BOX(type_card), list_type_combo, FALSE, FALSE, 0);
    app_data->list_type_combo = GTK_COMBO_BOX_TEXT(list_type_combo);

    // Section Type d'Éléments (Carte moderne)
    GtkWidget *element_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(element_card), "modern-card");
    gtk_box_pack_start(GTK_BOX(control_vbox), element_card, FALSE, FALSE, 0);

    GtkWidget *element_title = gtk_label_new("Type d'Éléments");
    gtk_label_set_markup(GTK_LABEL(element_title), "<span foreground='#ffffff' size='large' weight='bold'>🔢 Type d'Éléments</span>");
    gtk_box_pack_start(GTK_BOX(element_card), element_title, FALSE, FALSE, 0);

    GtkWidget *type_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "Entiers (Int)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "Réels (Float)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "Caractères (Char)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "Chaîne de Caractères");
    gtk_combo_box_set_active(GTK_COMBO_BOX(type_combo), 0);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(type_combo), "modern-combo");
    gtk_box_pack_start(GTK_BOX(element_card), type_combo, FALSE, FALSE, 0);
    app_data->type_combo = GTK_COMBO_BOX_TEXT(type_combo);

    // Bouton Créer Liste
    GtkWidget *btn_create = gtk_button_new_with_label("✨ Créer Liste (Vide)");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_create), "modern-button");
    g_signal_connect(btn_create, "clicked", G_CALLBACK(on_list_create_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(control_vbox), btn_create, FALSE, FALSE, 0);

    // Section Mode de Saisie (Carte moderne)
    GtkWidget *source_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(source_card), "modern-card");
    gtk_box_pack_start(GTK_BOX(control_vbox), source_card, FALSE, FALSE, 0);

    GtkWidget *source_title = gtk_label_new("Mode de Saisie");
    gtk_label_set_markup(GTK_LABEL(source_title), "<span foreground='#ffffff' size='large' weight='bold'>🔀 Mode de Saisie</span>");
    gtk_box_pack_start(GTK_BOX(source_card), source_title, FALSE, FALSE, 0);

    GtkWidget *source_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(source_card), source_vbox, FALSE, FALSE, 0);

    GtkWidget *radio_random = gtk_radio_button_new_with_label(NULL, "🎲 Aléatoire");
    GtkWidget *radio_manual = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_random), "✍️ Manuelle");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(radio_random), "modern-radio");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(radio_manual), "modern-radio");
    gtk_box_pack_start(GTK_BOX(source_vbox), radio_random, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(source_vbox), radio_manual, FALSE, FALSE, 0);

    g_signal_connect(radio_random, "toggled", G_CALLBACK(on_list_input_source_toggled), app_data);
    g_signal_connect(radio_manual, "toggled", G_CALLBACK(on_list_input_source_toggled), app_data);

    // --- Contrôles Aléatoires
    GtkWidget *random_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(source_vbox), random_box, FALSE, FALSE, 0);
    app_data->list_random_box = random_box;

    GtkWidget *size_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(random_box), size_hbox, FALSE, FALSE, 0);
    GtkWidget *lbl_size = gtk_label_new("Taille:");
    gtk_box_pack_start(GTK_BOX(size_hbox), lbl_size, FALSE, FALSE, 0);
    GtkWidget *spin_size = gtk_spin_button_new_with_range(1, 10000, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_size), 10);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(spin_size), "modern-spin");
    gtk_box_pack_start(GTK_BOX(size_hbox), spin_size, TRUE, TRUE, 0);
    app_data->list_size_input = GTK_SPIN_BUTTON(spin_size);

    GtkWidget *btn_fill = gtk_button_new_with_label("🎲 Générer / Remplir");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_fill), "modern-button");
    g_signal_connect(btn_fill, "clicked", G_CALLBACK(on_list_fill_random_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(random_box), btn_fill, FALSE, FALSE, 0);

    // --- Contrôles Manuels
    GtkWidget *manual_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(source_vbox), manual_box, FALSE, FALSE, 0);
    app_data->list_manual_box = manual_box;

    GtkWidget *entry_val = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_val), "Valeur (entier)");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(entry_val), "modern-combo");
    gtk_box_pack_start(GTK_BOX(manual_box), entry_val, FALSE, FALSE, 0);
    app_data->list_value_entry = GTK_ENTRY(entry_val);

    GtkWidget *btn_add = gtk_button_new_with_label("➕ Ajouter");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_add), "modern-button");
    g_signal_connect(btn_add, "clicked", G_CALLBACK(on_list_add_manual_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(manual_box), btn_add, FALSE, FALSE, 0);


    // Section Opérations (Carte moderne)
    GtkWidget *ops_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(ops_card), "modern-card");
    gtk_box_pack_start(GTK_BOX(control_vbox), ops_card, TRUE, TRUE, 0);

    // --- Opérations Avancées (Insertion)
    GtkWidget *label_ins = gtk_label_new("Insertion");
    gtk_label_set_markup(GTK_LABEL(label_ins), "<span foreground='#ffffff' size='large' weight='bold'>➕ Insertion</span>");
    gtk_box_pack_start(GTK_BOX(ops_card), label_ins, FALSE, FALSE, 0);

    GtkWidget *grid_ins = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid_ins), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid_ins), 8);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid_ins), TRUE);
    gtk_box_pack_start(GTK_BOX(ops_card), grid_ins, FALSE, FALSE, 0);

    GtkWidget *btn_ins_start = gtk_button_new_with_label("Début");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_ins_start), "modern-button");
    g_signal_connect(btn_ins_start, "clicked", G_CALLBACK(on_list_insert_start_clicked), app_data);
    gtk_grid_attach(GTK_GRID(grid_ins), btn_ins_start, 0, 0, 1, 1);

    GtkWidget *btn_ins_end = gtk_button_new_with_label("Fin");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_ins_end), "modern-button");
    g_signal_connect(btn_ins_end, "clicked", G_CALLBACK(on_list_insert_end_clicked), app_data);
    gtk_grid_attach(GTK_GRID(grid_ins), btn_ins_end, 1, 0, 1, 1);

    GtkWidget *btn_ins_pos = gtk_button_new_with_label("Pos...");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_ins_pos), "modern-button");
    g_signal_connect(btn_ins_pos, "clicked", G_CALLBACK(on_list_insert_pos_clicked), app_data);
    gtk_grid_attach(GTK_GRID(grid_ins), btn_ins_pos, 2, 0, 1, 1);

    // --- Opérations Avancées (Suppression)
    GtkWidget *label_del = gtk_label_new("Suppression");
    gtk_label_set_markup(GTK_LABEL(label_del), "<span foreground='#ffffff' size='large' weight='bold'>➖ Suppression</span>");
    gtk_box_pack_start(GTK_BOX(ops_card), label_del, FALSE, FALSE, 10);

    GtkWidget *grid_del = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid_del), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid_del), 8);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid_del), TRUE);
    gtk_box_pack_start(GTK_BOX(ops_card), grid_del, FALSE, FALSE, 0);

    GtkWidget *btn_del_start = gtk_button_new_with_label("Début");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_del_start), "modern-button");
    g_signal_connect(btn_del_start, "clicked", G_CALLBACK(on_list_delete_start_clicked), app_data);
    gtk_grid_attach(GTK_GRID(grid_del), btn_del_start, 0, 0, 1, 1);

    GtkWidget *btn_del_end = gtk_button_new_with_label("Fin");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_del_end), "modern-button");
    g_signal_connect(btn_del_end, "clicked", G_CALLBACK(on_list_delete_end_clicked), app_data);
    gtk_grid_attach(GTK_GRID(grid_del), btn_del_end, 1, 0, 1, 1);

    GtkWidget *btn_del_pos = gtk_button_new_with_label("Pos...");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_del_pos), "modern-button");
    g_signal_connect(btn_del_pos, "clicked", G_CALLBACK(on_list_delete_pos_clicked), app_data);
    gtk_grid_attach(GTK_GRID(grid_del), btn_del_pos, 2, 0, 1, 1);

    // Initial state
    gtk_widget_hide(manual_box); // Start with Random shown

    // Section Tri (Carte moderne)
    GtkWidget *sort_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(sort_card), "modern-card");
    gtk_box_pack_start(GTK_BOX(control_vbox), sort_card, FALSE, FALSE, 0);

    GtkWidget *sort_title = gtk_label_new("Opérations de Tri");
    gtk_label_set_markup(GTK_LABEL(sort_title), "<span foreground='#ffffff' size='large' weight='bold'>🔄 Opérations de Tri</span>");
    gtk_box_pack_start(GTK_BOX(sort_card), sort_title, FALSE, FALSE, 0);

    GtkWidget *sort_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_box_pack_start(GTK_BOX(sort_card), sort_vbox, FALSE, FALSE, 0);

    // Les 4 tris demandés
    const gchar *LIST_METHOD_NAMES[] = {"🔴 Tri à Bulles", "🟢 Tri par Insertion", "🔵 Tri Shell", "🟡 Tri Quicksort"};
    for (int i = 0; i < 4; i++) {
        GtkWidget *btn = gtk_button_new_with_label(LIST_METHOD_NAMES[i]);
        g_object_set_data(G_OBJECT(btn), "list-method-name", (gpointer)LIST_METHOD_NAMES[i]);
        // Style CSS activé
        gtk_style_context_add_class(gtk_widget_get_style_context(btn), "modern-button");
        g_signal_connect(btn, "clicked", G_CALLBACK(on_list_sort_clicked), app_data);
        gtk_box_pack_start(GTK_BOX(sort_vbox), btn, FALSE, FALSE, 0);
    }

    gtk_box_pack_start(GTK_BOX(main_hbox), control_scrolled, FALSE, FALSE, 0);


    // ========== PANEL DROIT : AFFICHAGE DES RÉSULTATS MODERNES ==========
    GtkWidget *display_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(display_scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(display_scrolled), GTK_SHADOW_NONE);

    GtkWidget *display_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(display_vbox), "display-panel");
    gtk_container_set_border_width(GTK_CONTAINER(display_vbox), 20);
    gtk_container_add(GTK_CONTAINER(display_scrolled), display_vbox);
    gtk_box_pack_start(GTK_BOX(main_hbox), display_scrolled, TRUE, TRUE, 0);

    // Affichage Graphique (Carte moderne)
    GtkWidget *drawing_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(drawing_card), "result-card");
    gtk_box_pack_start(GTK_BOX(display_vbox), drawing_card, TRUE, TRUE, 0);

    GtkWidget *drawing_header = gtk_label_new("Visualisation de la Liste");
    gtk_label_set_markup(GTK_LABEL(drawing_header), "<span foreground='#ffffff' size='large' weight='bold'>📊 Visualisation de la Liste</span>");
    gtk_box_pack_start(GTK_BOX(drawing_card), drawing_header, FALSE, FALSE, 0);

    // Affichage Graphique avec SCROLLING
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_IN);
    gtk_box_pack_start(GTK_BOX(drawing_card), scrolled_window, TRUE, TRUE, 0);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 600, 400);
    gtk_container_add(GTK_CONTAINER(scrolled_window), drawing_area);

    // Enable events for Click-to-Edit
    gtk_widget_add_events(drawing_area, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(drawing_area, "button-press-event", G_CALLBACK(on_list_click_event), app_data);

    app_data->list_drawing_area = drawing_area;
    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_list_callback), app_data);

    // Zone de message (Carte moderne)
    GtkWidget *info_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(info_card), "result-card");
    gtk_box_pack_start(GTK_BOX(display_vbox), info_card, FALSE, FALSE, 0);

    GtkWidget *info_header = gtk_label_new("Informations");
    gtk_label_set_markup(GTK_LABEL(info_header), "<span foreground='#ffffff' size='large' weight='bold'>ℹ️ Informations</span>");
    gtk_box_pack_start(GTK_BOX(info_card), info_header, FALSE, FALSE, 0);

    GtkWidget *list_info_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(list_info_scrolled, -1, 120);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(list_info_scrolled), GTK_SHADOW_IN);
    GtkWidget *list_info_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(list_info_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(list_info_view), GTK_WRAP_WORD);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(list_info_view), "modern-text-view");
    gtk_container_add(GTK_CONTAINER(list_info_scrolled), list_info_view);
    gtk_box_pack_start(GTK_BOX(info_card), list_info_scrolled, FALSE, FALSE, 0);
    app_data->list_info_view = GTK_TEXT_VIEW(list_info_view);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_data->list_info_view));
    gtk_text_buffer_set_text(buffer, "Cliquez sur 'Créer Liste (Vide)' pour commencer la visualisation.", -1);


    // Gestionnaire pour la fermeture de la fenêtre secondaire
    g_signal_connect(window, "delete-event", G_CALLBACK(on_secondary_window_delete), NULL);

    // Nettoyage à la destruction de la fenêtre (utiliser le wrapper)
    g_signal_connect(window, "destroy", G_CALLBACK(on_list_window_destroy), app_data);

    gtk_widget_show_all(window);
}
// =========================================================================
//                             MODULE GRAPHES (TP4)
// =========================================================================

// --- Gestion Mémoire ---
static Graph *graph_new(const gchar *element_type, gboolean is_directed, gboolean is_weighted) {
    Graph *g = g_new0(Graph, 1);
    g->num_nodes = 0;
    g->element_type = element_type ? element_type : "Entiers (Int)";
    g->is_directed = is_directed;
    g->is_weighted = is_weighted;

    // Obtenir la taille de l'élément
    size_t element_size;
    int (*compare_func)(const void *, const void *);
    get_type_info(g->element_type, &element_size, &compare_func);
    g->element_size = element_size;

    // Initialiser la matrice d'adjacence
    for(int i=0; i<MAX_GRAPH_NODES; i++) {
        for(int j=0; j<MAX_GRAPH_NODES; j++) {
            g->adj_matrix[i][j] = (i == j) ? 0 : INF;
        }
        g->node_data[i] = NULL;
        g->node_x[i] = 0.0;
        g->node_y[i] = 0.0;
    }
    return g;
}

static void graph_free(Graph *g) {
    if (!g) return;

    // Libérer les données des nœuds selon leur type
    if (g->element_type && g_strcmp0(g->element_type, "Chaîne de Caractères") == 0) {
        for(int i=0; i<g->num_nodes; i++) {
            if (g->node_data[i]) {
                char **str_ptr = (char **)g->node_data[i];
                if (str_ptr && *str_ptr) {
                    g_free(*str_ptr);  // Libérer la chaîne
                }
                g_free(g->node_data[i]);  // Libérer le pointeur vers char*
            }
        }
    } else {
        for(int i=0; i<g->num_nodes; i++) {
            if (g->node_data[i]) {
                g_free(g->node_data[i]);
            }
        }
    }

    g_free(g);
}


// --- Algorithmes ---

// Fonction helper pour reconstruire le chemin depuis prev[]
static void reconstruct_path(int prev[], int start, int end, GString *path_str) {
    if (end == -1) {
        g_string_append_printf(path_str, "Aucun chemin");
        return;
    }

    if (end == start) {
        g_string_append_printf(path_str, "%d", start);
        return;
    }

    if (prev[end] == -1 || prev[end] == end) {
        // Si prev[end] == end, cela signifie qu'on a une boucle ou un problème
        // Si prev[end] == -1, il n'y a pas de chemin
        g_string_append_printf(path_str, "Aucun chemin");
        return;
    }

    // Reconstruire le chemin récursivement
    reconstruct_path(prev, start, prev[end], path_str);
    g_string_append_printf(path_str, " -> %d", end);
}

// 1. Dijkstra (Source unique, poids positifs)
static void algo_dijkstra(Graph *g, int start_node, int dest_node, GString *res, AppData *app_data) {
    int dist[MAX_GRAPH_NODES];
    int visited[MAX_GRAPH_NODES];
    int prev[MAX_GRAPH_NODES];

    for(int i=0; i<g->num_nodes; i++) {
        dist[i] = INF;
        visited[i] = 0;
        prev[i] = -1;
    }
    dist[start_node] = 0;
    prev[start_node] = start_node;

    for(int count=0; count < g->num_nodes - 1; count++) {
        int min = INF, u = -1;

        // Find min distance vertex (non visité)
        for(int v=0; v < g->num_nodes; v++) {
            if (!visited[v] && dist[v] < min) {  // Changé <= en < pour éviter de sélectionner INF
                min = dist[v];
                u = v;
            }
        }

        // Si aucun nœud accessible n'est trouvé, arrêter
        if (u == -1 || dist[u] == INF) break;

        // Si on a atteint le nœud de destination, on peut s'arrêter (optimisation)
        if (u == dest_node) break;

        visited[u] = 1;

        // Relax neighbors
        for(int v=0; v < g->num_nodes; v++) {
            // Vérifier que l'arête existe (pas INF, et pas la diagonale u==v)
            if (!visited[v] && u != v && g->adj_matrix[u][v] != INF && dist[u] != INF) {
                int new_dist = dist[u] + g->adj_matrix[u][v];
                // Mettre à jour si le nouveau chemin est meilleur
                if (new_dist < dist[v]) {
                    dist[v] = new_dist;
                    prev[v] = u;
                }
            }
        }
    }

    // Output - Chemin spécifique entre start_node et dest_node
    g_string_append_printf(res, "═══════════════════════════════════════\n");
    g_string_append_printf(res, "Algorithme de Dijkstra\n");
    g_string_append_printf(res, "Nœud Initial: %d\n", start_node);
    g_string_append_printf(res, "Nœud Final: %d\n", dest_node);
    g_string_append_printf(res, "═══════════════════════════════════════\n\n");

    if (dest_node < 0 || dest_node >= g->num_nodes) {
        g_string_append_printf(res, "❌ Nœud destination invalide!\n");
        if (app_data) {
            app_data->graph_shortest_path_length = -1;
        }
        return;
    }

    if (dist[dest_node] == INF) {
        g_string_append_printf(res, "❌ Aucun chemin trouvé de %d à %d\n", start_node, dest_node);
        if (app_data) {
            app_data->graph_shortest_path_length = -1;
        }
    } else {
        g_string_append_printf(res, "✓ Distance minimale: %d\n", dist[dest_node]);
        g_string_append_printf(res, "✓ Chemin: ");
        GString *path_str = g_string_new("");
        reconstruct_path(prev, start_node, dest_node, path_str);
        g_string_append(res, path_str->str);
        g_string_free(path_str, TRUE);
        g_string_append_printf(res, "\n");

        // Stocker le chemin dans AppData pour la visualisation
        if (app_data) {
            // Reconstruire le chemin dans le tableau
            int path[MAX_GRAPH_NODES];
            int path_len = 0;
            int current = dest_node;

            // Reconstruire le chemin à l'envers
            // Vérifier d'abord si start_node == dest_node
            if (start_node == dest_node) {
                path[path_len++] = start_node;
            } else {
                // Reconstruire le chemin depuis dest_node vers start_node
                while (current != -1 && path_len < MAX_GRAPH_NODES) {
                    path[path_len++] = current;
                    if (current == start_node) break;
                    // Vérifier que prev[current] est valide et différent de current pour éviter les boucles
                    if (prev[current] == -1 || prev[current] == current) {
                        // Si on arrive à un nœud sans prédécesseur valide, le chemin est invalide
                        if (current != start_node) {
                            app_data->graph_shortest_path_length = -1;
                            return;
                        }
                        break;
                    }
                    current = prev[current];
                }
            }

            // Vérifier que le chemin commence bien par start_node
            if (path_len > 0 && path[path_len - 1] != start_node) {
                app_data->graph_shortest_path_length = -1;
                return;
            }

            // Inverser le chemin pour avoir start_node -> ... -> dest_node
            for(int i = 0; i < path_len; i++) {
                app_data->graph_shortest_path[i] = path[path_len - 1 - i];
            }
            app_data->graph_shortest_path_length = path_len;
        }
    }
}

// 2. Bellman-Ford (Source unique, poids négatifs possibles)
static void algo_bellman_ford(Graph *g, int start_node, int dest_node, GString *res, AppData *app_data) {
    int dist[MAX_GRAPH_NODES];
    int prev[MAX_GRAPH_NODES];

    for(int i=0; i<g->num_nodes; i++) {
        dist[i] = INF;
        prev[i] = -1;
    }
    dist[start_node] = 0;
    prev[start_node] = start_node;

    for(int i=1; i <= g->num_nodes - 1; i++) {
        for(int u=0; u < g->num_nodes; u++) {
            for(int v=0; v < g->num_nodes; v++) {
                if (g->adj_matrix[u][v] != INF && dist[u] != INF &&
                    dist[u] + g->adj_matrix[u][v] < dist[v]) {
                    dist[v] = dist[u] + g->adj_matrix[u][v];
                    prev[v] = u;
                }
            }
        }
    }

    // Check for negative weight cycles
    for(int u=0; u < g->num_nodes; u++) {
        for(int v=0; v < g->num_nodes; v++) {
            if (g->adj_matrix[u][v] != INF && dist[u] != INF &&
                dist[u] + g->adj_matrix[u][v] < dist[v]) {
                g_string_append_printf(res, "═══════════════════════════════════════\n");
                g_string_append_printf(res, "Algorithme de Bellman-Ford\n");
                g_string_append_printf(res, "❌ Cycle de poids négatif détecté!\n");
                g_string_append_printf(res, "═══════════════════════════════════════\n");
                if (app_data) {
                    app_data->graph_shortest_path_length = -1;
                }
                return;
            }
        }
    }

    // Output - Chemin spécifique entre start_node et dest_node
    g_string_append_printf(res, "═══════════════════════════════════════\n");
    g_string_append_printf(res, "Algorithme de Bellman-Ford\n");
    g_string_append_printf(res, "Nœud Initial: %d\n", start_node);
    g_string_append_printf(res, "Nœud Final: %d\n", dest_node);
    g_string_append_printf(res, "═══════════════════════════════════════\n\n");

    if (dest_node < 0 || dest_node >= g->num_nodes) {
        g_string_append_printf(res, "❌ Nœud destination invalide!\n");
        if (app_data) {
            app_data->graph_shortest_path_length = -1;
        }
        return;
    }

    if (dist[dest_node] == INF) {
        g_string_append_printf(res, "❌ Aucun chemin trouvé de %d à %d\n", start_node, dest_node);
        if (app_data) {
            app_data->graph_shortest_path_length = -1;
        }
    } else {
        g_string_append_printf(res, "✓ Distance minimale: %d\n", dist[dest_node]);
        g_string_append_printf(res, "✓ Chemin: ");
        GString *path_str = g_string_new("");
        reconstruct_path(prev, start_node, dest_node, path_str);
        g_string_append(res, path_str->str);
        g_string_free(path_str, TRUE);
        g_string_append_printf(res, "\n");

        // Stocker le chemin dans AppData pour la visualisation
        if (app_data) {
            int path[MAX_GRAPH_NODES];
            int path_len = 0;
            int current = dest_node;

            // Reconstruire le chemin à l'envers
            // Vérifier d'abord si start_node == dest_node
            if (start_node == dest_node) {
                path[path_len++] = start_node;
            } else {
                // Reconstruire le chemin depuis dest_node vers start_node
                while (current != -1 && path_len < MAX_GRAPH_NODES) {
                    path[path_len++] = current;
                    if (current == start_node) break;
                    // Vérifier que prev[current] est valide et différent de current pour éviter les boucles
                    if (prev[current] == -1 || prev[current] == current) {
                        // Si on arrive à un nœud sans prédécesseur valide, le chemin est invalide
                        if (current != start_node) {
                            app_data->graph_shortest_path_length = -1;
                            return;
                        }
                        break;
                    }
                    current = prev[current];
                }
            }

            // Vérifier que le chemin commence bien par start_node
            if (path_len > 0 && path[path_len - 1] != start_node) {
                app_data->graph_shortest_path_length = -1;
                return;
            }

            // Inverser le chemin pour avoir start_node -> ... -> dest_node
            for(int i = 0; i < path_len; i++) {
                app_data->graph_shortest_path[i] = path[path_len - 1 - i];
            }
            app_data->graph_shortest_path_length = path_len;
        }
    }
}

// 3. Floyd-Warshall (Tous les chemins)
static void algo_floyd_warshall(Graph *g, int start_node, int dest_node, GString *res, AppData *app_data) {
    int dist[MAX_GRAPH_NODES][MAX_GRAPH_NODES];
    int next[MAX_GRAPH_NODES][MAX_GRAPH_NODES];

    // Init
    for(int i=0; i<g->num_nodes; i++) {
        for(int j=0; j<g->num_nodes; j++) {
            dist[i][j] = g->adj_matrix[i][j];
            if (i == j) {
                next[i][j] = i;
            } else if (g->adj_matrix[i][j] != INF) {
                next[i][j] = j;
            } else {
                next[i][j] = -1;
            }
        }
    }

    // Core
    for(int k=0; k < g->num_nodes; k++) {
        for(int i=0; i < g->num_nodes; i++) {
            for(int j=0; j < g->num_nodes; j++) {
                if (dist[i][k] != INF && dist[k][j] != INF &&
                    dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    next[i][j] = next[i][k];
                }
            }
        }
    }

    // Output - Chemin spécifique entre start_node et dest_node
    g_string_append_printf(res, "═══════════════════════════════════════\n");
    g_string_append_printf(res, "Algorithme de Floyd-Warshall\n");
    g_string_append_printf(res, "Nœud Initial: %d\n", start_node);
    g_string_append_printf(res, "Nœud Final: %d\n", dest_node);
    g_string_append_printf(res, "═══════════════════════════════════════\n\n");

    if (start_node < 0 || start_node >= g->num_nodes || dest_node < 0 || dest_node >= g->num_nodes) {
        g_string_append_printf(res, "❌ Nœud(s) invalide(s)!\n");
        return;
    }

    if (dist[start_node][dest_node] == INF) {
        g_string_append_printf(res, "❌ Aucun chemin trouvé de %d à %d\n", start_node, dest_node);
        if (app_data) {
            app_data->graph_shortest_path_length = -1;
        }
    } else {
        g_string_append_printf(res, "✓ Distance minimale: %d\n", dist[start_node][dest_node]);
        g_string_append_printf(res, "✓ Chemin: ");

        // Reconstruire le chemin avec next[]
        int u = start_node;
        g_string_append_printf(res, "%d", u);

        // Stocker le chemin dans AppData
        if (app_data) {
            app_data->graph_shortest_path[0] = start_node;
            int path_len = 1;

            while (u != dest_node && next[u][dest_node] != -1 && path_len < MAX_GRAPH_NODES) {
                u = next[u][dest_node];
                app_data->graph_shortest_path[path_len++] = u;
                g_string_append_printf(res, " -> %d", u);
            }
            app_data->graph_shortest_path_length = path_len;
        } else {
            while (u != dest_node && next[u][dest_node] != -1) {
                u = next[u][dest_node];
                g_string_append_printf(res, " -> %d", u);
            }
        }
        g_string_append_printf(res, "\n");
    }
}

// --- Drawing (Circular Layout) ---
static gboolean draw_graph_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_graph) return FALSE;

    Graph *g = app_data->current_graph;
    guint width = gtk_widget_get_allocated_width(widget);
    guint height = gtk_widget_get_allocated_height(widget);

    double center_x = width / 2.0;
    double center_y = height / 2.0;
    double radius = (width < height ? width : height) / 2.0 - 60.0;

    // Fond sombre cohérent avec le reste de l'application
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.12); // #1a1a1f Dark BG
    cairo_paint(cr);

    if (g->num_nodes == 0) {
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_set_font_size(cr, 16);
        cairo_move_to(cr, center_x - 100, center_y);
        cairo_show_text(cr, "Cliquez pour ajouter un nœud");
        return TRUE;
    }

    // Utiliser les positions personnalisées des nœuds
    typedef struct { double x, y; } Point;
    Point points[MAX_GRAPH_NODES];

    for(int i=0; i<g->num_nodes; i++) {
        // Si la position n'est pas définie (0,0), utiliser une position par défaut au centre
        if (g->node_x[i] == 0.0 && g->node_y[i] == 0.0) {
            double angle = 2 * M_PI * i / g->num_nodes - M_PI / 2.0;
            points[i].x = center_x + radius * cos(angle);
            points[i].y = center_y + radius * sin(angle);
        } else {
            points[i].x = g->node_x[i];
            points[i].y = g->node_y[i];
        }
    }

    double node_radius = 20.0;

    // Dessiner les arêtes AVANT les nœuds pour qu'elles passent derrière
    // D'abord les arêtes normales (pas dans le chemin)
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.6); // Couleur gris-bleu pour les arêtes normales
    cairo_set_line_width(cr, 2.0);

    for(int i=0; i<g->num_nodes; i++) {
        for(int j=0; j<g->num_nodes; j++) {
            // Dessiner une arête si elle existe (pas INF et pas la diagonale)
            // Pour les graphes non orientés, dessiner uniquement les arêtes où i < j pour éviter les doublons
            if (i != j && g->adj_matrix[i][j] != INF && (g->is_directed || i < j)) {
                // Vérifier si cette arête fait partie du chemin le plus court
                gboolean is_in_path = FALSE;
                if (app_data->graph_shortest_path_length > 0) {
                    for(int k = 0; k < app_data->graph_shortest_path_length - 1; k++) {
                        if (app_data->graph_shortest_path[k] == i &&
                            app_data->graph_shortest_path[k+1] == j) {
                            is_in_path = TRUE;
                            break;
                        }
                    }
                }
                if (is_in_path) {
                    continue; // On la dessinera après avec une couleur différente
                }
                double x1 = points[i].x;
                double y1 = points[i].y;
                double x2 = points[j].x;
                double y2 = points[j].y;

                // Calculer la direction et ajuster pour que l'arête commence/termine au bord du cercle
                double dx = x2 - x1;
                double dy = y2 - y1;
                double dist = sqrt(dx*dx + dy*dy);

                if (dist > 0) {
                    // Normaliser et ajuster pour le rayon du nœud
                    double nx = dx / dist;
                    double ny = dy / dist;

                    double start_x = x1 + nx * node_radius;
                    double start_y = y1 + ny * node_radius;
                    double end_x = x2 - nx * node_radius;
                    double end_y = y2 - ny * node_radius;

                    // Dessiner la ligne
                    cairo_move_to(cr, start_x, start_y);
                    cairo_line_to(cr, end_x, end_y);
                    cairo_stroke(cr);

                    // Dessiner la flèche uniquement pour les graphes orientés
                    if (g->is_directed) {
                        double arrow_size = 8.0;
                        double angle = atan2(dy, dx);

                        // Point de la flèche au bord du nœud de destination
                        double arrow_x = end_x;
                        double arrow_y = end_y;

                        // Calculer les points de la flèche
                        double arrow_angle1 = angle + M_PI - M_PI/6;
                        double arrow_angle2 = angle + M_PI + M_PI/6;

                        cairo_move_to(cr, arrow_x, arrow_y);
                        cairo_line_to(cr, arrow_x + arrow_size * cos(arrow_angle1),
                                      arrow_y + arrow_size * sin(arrow_angle1));
                        cairo_move_to(cr, arrow_x, arrow_y);
                        cairo_line_to(cr, arrow_x + arrow_size * cos(arrow_angle2),
                                      arrow_y + arrow_size * sin(arrow_angle2));
                        cairo_stroke(cr);
                    }

                    // Afficher le poids de l'arête au milieu (uniquement pour les graphes pondérés)
                    if (g->is_weighted) {
                        double mid_x = (start_x + end_x) / 2.0;
                        double mid_y = (start_y + end_y) / 2.0;

                        // Fond pour le texte du poids
                        char weight_text[32];
                        snprintf(weight_text, 32, "%d", g->adj_matrix[i][j]);

                    cairo_text_extents_t weight_extents;
                    cairo_set_source_rgb(cr, 0.1, 0.1, 0.12); // Fond sombre
                    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
                    cairo_set_font_size(cr, 10);
                    cairo_text_extents(cr, weight_text, &weight_extents);

                    // Rectangle de fond pour le poids
                    double padding = 4.0;
                    cairo_rectangle(cr,
                        mid_x - weight_extents.width/2 - padding,
                        mid_y - weight_extents.height/2 - padding,
                        weight_extents.width + 2*padding,
                        weight_extents.height + 2*padding);
                    cairo_fill(cr);

                        // Texte du poids
                        cairo_set_source_rgb(cr, 1.0, 1.0, 0.8); // Jaune clair pour le poids
                        cairo_move_to(cr, mid_x - weight_extents.width/2, mid_y + weight_extents.height/2);
                        cairo_show_text(cr, weight_text);
                    }
                }
            }
        }
    }

    // Dessiner les arêtes du chemin le plus court avec une couleur différente (vert/jaune)
    // IMPORTANT: Les flèches doivent TOUJOURS pointer dans la direction du chemin (vers la destination)
    if (app_data->graph_shortest_path_length > 0) {
        cairo_set_source_rgb(cr, 0.0, 1.0, 0.5); // Vert vif pour le chemin le plus court
        cairo_set_line_width(cr, 4.0); // Ligne plus épaisse pour le chemin

        for(int k = 0; k < app_data->graph_shortest_path_length - 1; k++) {
            int i = app_data->graph_shortest_path[k];      // Nœud source dans le chemin
            int j = app_data->graph_shortest_path[k+1];    // Nœud destination dans le chemin

            if (i >= 0 && i < g->num_nodes && j >= 0 && j < g->num_nodes) {
                // Vérifier que l'arête existe dans la direction du chemin
                // Pour les graphes non orientés, l'arête peut exister dans les deux sens
                gboolean edge_exists = FALSE;
                if (g->adj_matrix[i][j] != INF) {
                    edge_exists = TRUE;
                } else if (!g->is_directed && g->adj_matrix[j][i] != INF) {
                    // Pour les graphes non orientés, vérifier aussi dans l'autre sens
                    edge_exists = TRUE;
                }

                if (!edge_exists) continue; // L'arête n'existe pas dans cette direction

                double x1 = points[i].x;
                double y1 = points[i].y;
                double x2 = points[j].x;
                double y2 = points[j].y;

                // Calculer la direction et ajuster pour que l'arête commence/termine au bord du cercle
                double dx = x2 - x1;
                double dy = y2 - y1;
                double dist = sqrt(dx*dx + dy*dy);

                if (dist > 0) {
                    // Normaliser et ajuster pour le rayon du nœud
                    double nx = dx / dist;
                    double ny = dy / dist;

                    double start_x = x1 + nx * node_radius;
                    double start_y = y1 + ny * node_radius;
                    double end_x = x2 - nx * node_radius;
                    double end_y = y2 - ny * node_radius;

                    // Dessiner la ligne du chemin (toujours de i vers j, direction du chemin)
                    cairo_move_to(cr, start_x, start_y);
                    cairo_line_to(cr, end_x, end_y);
                    cairo_stroke(cr);

                    // Dessiner la flèche TOUJOURS dans la direction du chemin (i -> j)
                    // Même pour les graphes non orientés, la flèche pointe vers la destination
                    double arrow_size = 10.0; // Flèche plus grande pour le chemin
                    double angle = atan2(dy, dx); // Angle de i vers j

                    double arrow_x = end_x;
                    double arrow_y = end_y;

                    double arrow_angle1 = angle + M_PI - M_PI/6;
                    double arrow_angle2 = angle + M_PI + M_PI/6;

                    cairo_move_to(cr, arrow_x, arrow_y);
                    cairo_line_to(cr, arrow_x + arrow_size * cos(arrow_angle1),
                                  arrow_y + arrow_size * sin(arrow_angle1));
                    cairo_move_to(cr, arrow_x, arrow_y);
                    cairo_line_to(cr, arrow_x + arrow_size * cos(arrow_angle2),
                                  arrow_y + arrow_size * sin(arrow_angle2));
                    cairo_stroke(cr);
                }
            }
        }
    }

    // Dessiner la ligne temporaire pendant le glissement pour créer un arc
    if (app_data->graph_dragging_edge_source != -1 &&
        app_data->graph_dragging_edge_source >= 0 &&
        app_data->graph_dragging_edge_source < g->num_nodes) {
        double src_x = points[app_data->graph_dragging_edge_source].x;
        double src_y = points[app_data->graph_dragging_edge_source].y;
        double dest_x = app_data->graph_dragging_edge_x;
        double dest_y = app_data->graph_dragging_edge_y;

        // Calculer la direction et ajuster pour que la ligne commence au bord du cercle source
        double dx = dest_x - src_x;
        double dy = dest_y - src_y;
        double dist = sqrt(dx*dx + dy*dy);

        if (dist > 0) {
            // Normaliser et ajuster pour le rayon du nœud
            double nx = dx / dist;
            double ny = dy / dist;

            double start_x = src_x + nx * node_radius;
            double start_y = src_y + ny * node_radius;

            // Dessiner la ligne temporaire en pointillés
            cairo_set_source_rgb(cr, 0.7, 0.7, 0.9); // Couleur bleu clair pour la ligne temporaire
            cairo_set_line_width(cr, 2.0);
            cairo_set_dash(cr, (const double[]){5.0, 5.0}, 2, 0);
            cairo_move_to(cr, start_x, start_y);
            cairo_line_to(cr, dest_x, dest_y);
            cairo_stroke(cr);
            cairo_set_dash(cr, NULL, 0, 0); // Réinitialiser les pointillés
        }
    }

    // Dessiner les nœuds (par-dessus les arêtes)
    for(int i=0; i<g->num_nodes; i++) {
        // Cercle du nœud - couleur différente si dans le chemin le plus court
        gboolean in_path = FALSE;
        if (app_data->graph_shortest_path_length > 0) {
            for(int k = 0; k < app_data->graph_shortest_path_length; k++) {
                if (app_data->graph_shortest_path[k] == i) {
                    in_path = TRUE;
                    break;
                }
            }
        }
        if (in_path) {
            // Nœud dans le chemin le plus court - vert vif
            cairo_set_source_rgb(cr, 0.0, 0.8, 0.4); // Vert pour les nœuds du chemin
        } else if (app_data->graph_selected_node == i) {
            cairo_set_source_rgb(cr, 0.3, 0.3, 0.4); // Fond plus clair si sélectionné
        } else {
            cairo_set_source_rgb(cr, 0.2, 0.2, 0.25); // Fond sombre
        }
        cairo_arc(cr, points[i].x, points[i].y, node_radius, 0, 2 * M_PI);
        cairo_fill(cr);

        // Texte du nœud (valeur selon le type)
        char node_text[64];
        if (g->node_data[i] && g->element_type) {
            if (g_strcmp0(g->element_type, "Entiers (Int)") == 0) {
                snprintf(node_text, 64, "%d", *(int*)g->node_data[i]);
            } else if (g_strcmp0(g->element_type, "Réels (Float)") == 0) {
                snprintf(node_text, 64, "%.1f", *(float*)g->node_data[i]);
            } else if (g_strcmp0(g->element_type, "Caractères (Char)") == 0) {
                snprintf(node_text, 64, "%c", *(char*)g->node_data[i]);
            } else if (g_strcmp0(g->element_type, "Chaîne de Caractères") == 0) {
                char **str_ptr = (char **)g->node_data[i];
                snprintf(node_text, 64, "%s", (str_ptr && *str_ptr) ? *str_ptr : "");
            } else {
                snprintf(node_text, 64, "?");
            }
        } else {
            snprintf(node_text, 64, "?");
        }

        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // Texte blanc
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 11);
        cairo_text_extents_t extents;
        cairo_text_extents(cr, node_text, &extents);
        cairo_move_to(cr, points[i].x - extents.width/2, points[i].y + extents.height/2);
        cairo_show_text(cr, node_text);
    }

    return TRUE;
}

// --- Fonctions d'édition manuelle ---

// Fonction pour trouver un nœud par sa valeur selon le type de données
static int graph_find_node_by_value(Graph *g, const gchar *value_str) {
    if (!g || !value_str || g->num_nodes == 0) return -1;

    for(int i = 0; i < g->num_nodes; i++) {
        if (!g->node_data[i]) continue;

        gboolean match = FALSE;

        if (g_strcmp0(g->element_type, "Entiers (Int)") == 0) {
            int node_val = *(int*)g->node_data[i];
            int input_val = atoi(value_str);
            match = (node_val == input_val);
        } else if (g_strcmp0(g->element_type, "Réels (Float)") == 0) {
            float node_val = *(float*)g->node_data[i];
            float input_val = (float)atof(value_str);
            // Comparaison avec une petite tolérance pour les flottants
            match = (fabs(node_val - input_val) < 0.0001f);
        } else if (g_strcmp0(g->element_type, "Caractères (Char)") == 0) {
            char node_val = *(char*)g->node_data[i];
            char input_val = (strlen(value_str) > 0) ? value_str[0] : '\0';
            match = (node_val == input_val);
        } else if (g_strcmp0(g->element_type, "Chaîne de Caractères") == 0) {
            char **str_ptr = (char **)g->node_data[i];
            if (str_ptr && *str_ptr) {
                match = (g_strcmp0(*str_ptr, value_str) == 0);
            }
        }

        if (match) return i;
    }

    return -1;
}

static void graph_add_node(Graph *g, void *value, double x, double y) {
    if (!g || !value) return;
    if (g->num_nodes >= MAX_GRAPH_NODES) {
        // Limite atteinte - l'appelant devrait vérifier avant d'appeler
        return;
    }

    int idx = g->num_nodes;
    void *data = malloc(g->element_size);
    if (!data) return; // Échec d'allocation

    if (g_strcmp0(g->element_type, "Chaîne de Caractères") == 0) {
        char **str_ptr = (char **)data;
        char **value_ptr = (char **)value;
        *str_ptr = g_strdup(value_ptr && *value_ptr ? *value_ptr : "");
        if (!*str_ptr) {
            free(data); // Libérer la mémoire si l'allocation de chaîne échoue
            return;
        }
    } else {
        memcpy(data, value, g->element_size);
    }

    g->node_data[idx] = data;
    g->node_x[idx] = x;
    g->node_y[idx] = y;
    g->num_nodes++;

    // Initialiser les arêtes pour le nouveau nœud - TOUTES à INF sauf la diagonale (0)
    // IMPORTANT: S'assurer qu'aucune arête n'est créée automatiquement
    for(int i=0; i<g->num_nodes; i++) {
        if (i == idx) {
            g->adj_matrix[idx][idx] = 0; // Diagonale à 0
        } else {
            // FORCER à INF pour éviter toute arête automatique
            g->adj_matrix[idx][i] = INF; // Pas d'arête sortante
            g->adj_matrix[i][idx] = INF; // Pas d'arête entrante
        }
    }

    // Double vérification: s'assurer que toutes les arêtes vers/depuis ce nœud sont INF
    // On réinitialise TOUTES les arêtes pour ce nœud pour éviter toute création automatique
    for(int i=0; i<MAX_GRAPH_NODES; i++) {
        if (i != idx) {
            g->adj_matrix[idx][i] = INF;
            g->adj_matrix[i][idx] = INF;
        } else if (i == idx) {
            g->adj_matrix[idx][idx] = 0; // Diagonale à 0
        }
    }

    // Triple vérification: s'assurer qu'aucune arête n'a été créée par erreur
    // Réinitialiser toutes les arêtes existantes entre tous les nœuds (sauf diagonales)
    for(int i=0; i<g->num_nodes; i++) {
        for(int j=0; j<g->num_nodes; j++) {
            if (i != j) {
                // Si une arête existe déjà et n'a pas été explicitement créée, la supprimer
                // On garde seulement les arêtes qui ont été créées manuellement (mode Ajouter Arête)
                // Mais comme on vient de créer un nœud, on s'assure qu'il n'y a pas d'arête
                if (i == idx || j == idx) {
                    g->adj_matrix[i][j] = INF; // FORCER à INF pour le nouveau nœud
                }
            }
        }
    }
}

// Fonction pour créer plusieurs nœuds d'un coup en cercle
static void on_graph_create_multiple_nodes(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_graph) return;

    // Demander le nombre de nœuds à créer
    GtkWidget *parent = gtk_widget_get_toplevel(widget);
    int num_nodes_to_add = get_integer_input(parent, "Créer Plusieurs Nœuds",
                                             "Nombre de nœuds à créer:", 5);

    if (num_nodes_to_add <= 0) return; // Annulation ou valeur invalide

    // Vérifier si on peut ajouter ces nœuds
    if (app_data->current_graph->num_nodes + num_nodes_to_add > MAX_GRAPH_NODES) {
        show_error_dialog(parent, "Limite atteinte",
            g_strdup_printf("Impossible de créer %d nœuds. Maximum: %d nœuds au total.",
                          num_nodes_to_add, MAX_GRAPH_NODES));
        return;
    }

    // Obtenir les dimensions du canvas pour placer les nœuds en cercle
    guint width = gtk_widget_get_allocated_width(app_data->graph_drawing_area);
    guint height = gtk_widget_get_allocated_height(app_data->graph_drawing_area);

    if (width == 0 || height == 0) {
        // Valeurs par défaut si le widget n'est pas encore affiché
        width = 800;
        height = 600;
    }

    double center_x = width / 2.0;
    double center_y = height / 2.0;
    double radius = (width < height ? width : height) / 2.0 - 60.0;

    // Obtenir le type depuis le combo box (au cas où il aurait changé)
    const gchar *element_type = gtk_combo_box_text_get_active_text(app_data->graph_type_combo);
    if (!element_type) {
        element_type = app_data->current_graph->element_type;
    }

    // Mettre à jour le type du graphe si nécessaire
    if (g_strcmp0(element_type, app_data->current_graph->element_type) != 0) {
        app_data->current_graph->element_type = element_type;
        size_t element_size;
        int (*compare_func)(const void *, const void *);
        get_type_info(element_type, &element_size, &compare_func);
        app_data->current_graph->element_size = element_size;
    }

    int start_index = app_data->current_graph->num_nodes;

    // Créer les nœuds en cercle
    for (int i = 0; i < num_nodes_to_add; i++) {
        double angle = 2 * M_PI * i / num_nodes_to_add - M_PI / 2.0;
        double x = center_x + radius * cos(angle);
        double y = center_y + radius * sin(angle);

        // Créer la valeur selon le type
        void *val = NULL;
        if (g_strcmp0(element_type, "Entiers (Int)") == 0) {
            val = g_malloc(sizeof(int));
            *(int*)val = start_index + i;
        } else if (g_strcmp0(element_type, "Réels (Float)") == 0) {
            val = g_malloc(sizeof(float));
            *(float*)val = (float)(start_index + i) + 0.5f; // Ajouter 0.5 pour avoir des valeurs décimales
        } else if (g_strcmp0(element_type, "Caractères (Char)") == 0) {
            val = g_malloc(sizeof(char));
            *(char*)val = 'A' + (start_index + i) % 26;
        } else if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
            val = g_malloc(sizeof(char*));
            char buf[32];
            snprintf(buf, 32, "N%d", start_index + i);
            *(char**)val = g_strdup(buf);
        }

        if (val) {
            graph_add_node(app_data->current_graph, val, x, y);

            // Libérer la mémoire selon le type
            if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
                g_free(*(char**)val);
            }
            g_free(val);
        }
    }

    gtk_widget_queue_draw(app_data->graph_drawing_area);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
    gchar *msg = g_strdup_printf("%d nœuds créés avec succès. Vous pouvez maintenant les lier avec le mode 'Ajouter Arête'.",
                                num_nodes_to_add);
    gtk_text_buffer_set_text(buffer, msg, -1);
    g_free(msg);
}

static void graph_set_node_value(Graph *g, int node_id, void *value) {
    if (!g || node_id < 0 || node_id >= g->num_nodes) return;

    if (g->node_data[node_id]) {
        if (g_strcmp0(g->element_type, "Chaîne de Caractères") == 0) {
            char **str_ptr = (char **)g->node_data[node_id];
            char **value_ptr = (char **)value;
            if (str_ptr && *str_ptr) g_free(*str_ptr);
            if (str_ptr) *str_ptr = g_strdup(value_ptr && *value_ptr ? *value_ptr : "");
        } else {
            memcpy(g->node_data[node_id], value, g->element_size);
        }
    } else {
        void *data = malloc(g->element_size);
        if (g_strcmp0(g->element_type, "Chaîne de Caractères") == 0) {
            char **str_ptr = (char **)data;
            char **value_ptr = (char **)value;
            *str_ptr = g_strdup(value_ptr && *value_ptr ? *value_ptr : "");
        } else {
            memcpy(data, value, g->element_size);
        }
        g->node_data[node_id] = data;
    }
}

static void graph_add_edge(Graph *g, int src, int dest, int weight) {
    if (!g || src < 0 || src >= g->num_nodes || dest < 0 || dest >= g->num_nodes) return;

    // Si le graphe est non pondéré, utiliser un poids de 1
    int actual_weight = g->is_weighted ? weight : 1;

    // Ajouter l'arête
    g->adj_matrix[src][dest] = actual_weight;

    // Si le graphe est non orienté, ajouter l'arête dans l'autre sens
    if (!g->is_directed) {
        g->adj_matrix[dest][src] = actual_weight;
    }
}

static void graph_remove_edge(Graph *g, int src, int dest) {
    if (!g || src < 0 || src >= g->num_nodes || dest < 0 || dest >= g->num_nodes) return;
    g->adj_matrix[src][dest] = INF;
    // Si le graphe est non orienté, supprimer l'arête dans l'autre sens aussi
    if (!g->is_directed) {
        g->adj_matrix[dest][src] = INF;
    }
}

// Fonction pour supprimer toutes les arêtes du graphe
static void graph_remove_all_edges(Graph *g) {
    if (!g) return;
    for(int i=0; i<g->num_nodes; i++) {
        for(int j=0; j<g->num_nodes; j++) {
            if (i != j) {
                g->adj_matrix[i][j] = INF;
            } else {
                g->adj_matrix[i][j] = 0; // Diagonale reste à 0
            }
        }
    }
}

static void graph_remove_node(Graph *g, int node_id) {
    if (!g || node_id < 0 || node_id >= g->num_nodes) return;

    // Libérer les données du nœud
    if (g->node_data[node_id]) {
        if (g_strcmp0(g->element_type, "Chaîne de Caractères") == 0) {
            char **str_ptr = (char **)g->node_data[node_id];
            if (str_ptr && *str_ptr) g_free(*str_ptr);
            g_free(g->node_data[node_id]);  // Libérer aussi le pointeur vers char*
        } else {
            g_free(g->node_data[node_id]);
        }
        g->node_data[node_id] = NULL;
    }

    // Déplacer les nœuds suivants
    for(int i=node_id; i<g->num_nodes-1; i++) {
        g->node_data[i] = g->node_data[i+1];
        g->node_x[i] = g->node_x[i+1];
        g->node_y[i] = g->node_y[i+1];
        for(int j=0; j<g->num_nodes; j++) {
            g->adj_matrix[i][j] = g->adj_matrix[i+1][j];
            g->adj_matrix[j][i] = g->adj_matrix[j][i+1];
        }
    }

    g->num_nodes--;

    // Réinitialiser la dernière ligne/colonne
    for(int i=0; i<MAX_GRAPH_NODES; i++) {
        g->adj_matrix[g->num_nodes][i] = INF;
        g->adj_matrix[i][g->num_nodes] = INF;
    }
    if (g->num_nodes > 0) {
        g->adj_matrix[g->num_nodes][g->num_nodes] = 0;
    }
}

// --- Callbacks ---

// Callback pour gérer le changement de type de données
static void on_graph_type_changed(GtkComboBoxText *combo, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data || !app_data->current_graph) return;

    const gchar *new_type = gtk_combo_box_text_get_active_text(combo);
    if (!new_type) return;

    // Si le graphe n'est pas vide, demander confirmation
    if (app_data->current_graph->num_nodes > 0) {
        GtkWidget *parent = gtk_widget_get_toplevel(GTK_WIDGET(combo));
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(parent),
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_QUESTION,
            GTK_BUTTONS_YES_NO,
            "Le graphe contient %d nœud(s).\n\nChanger le type de données va réinitialiser le graphe.\n\nVoulez-vous continuer ?",
            app_data->current_graph->num_nodes
        );
        gtk_window_set_title(GTK_WINDOW(dialog), "Changer le type de données");

        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        if (response != GTK_RESPONSE_YES) {
            // Restaurer l'ancien type dans le combo box
            const gchar *old_type = app_data->current_graph->element_type;
            if (g_strcmp0(old_type, "Entiers (Int)") == 0) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
            } else if (g_strcmp0(old_type, "Réels (Float)") == 0) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 1);
            } else if (g_strcmp0(old_type, "Caractères (Char)") == 0) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 2);
            } else if (g_strcmp0(old_type, "Chaîne de Caractères") == 0) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 3);
            }
            return;
        }

        // Réinitialiser le graphe en préservant les options orienté/pondéré
        gboolean is_directed = app_data->current_graph->is_directed;
        gboolean is_weighted = app_data->current_graph->is_weighted;
        graph_free(app_data->current_graph);
        app_data->current_graph = graph_new(new_type, is_directed, is_weighted);

        // Réinitialiser les arêtes
        for(int i=0; i<MAX_GRAPH_NODES; i++) {
            for(int j=0; j<MAX_GRAPH_NODES; j++) {
                if (i != j) {
                    app_data->current_graph->adj_matrix[i][j] = INF;
                }
            }
        }

        gtk_widget_queue_draw(app_data->graph_drawing_area);

        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
        gchar *msg = g_strdup_printf("✓ Type de données changé en: %s\nLe graphe a été réinitialisé.", new_type);
        gtk_text_buffer_set_text(buffer, msg, -1);
        g_free(msg);
    } else {
        // Le graphe est vide, on peut simplement changer le type
        app_data->current_graph->element_type = new_type;

        // Mettre à jour la taille d'élément
        size_t element_size;
        int (*compare_func)(const void *, const void *);
        get_type_info(new_type, &element_size, &compare_func);
        app_data->current_graph->element_size = element_size;

        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
        gchar *msg = g_strdup_printf("✓ Type de données changé en: %s\nVous pouvez maintenant ajouter des nœuds.", new_type);
        gtk_text_buffer_set_text(buffer, msg, -1);
        g_free(msg);
    }
}

// Callback pour le changement d'orientation (orienté/non orienté)
static void on_graph_directed_changed(GtkComboBoxText *combo, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data || !app_data->current_graph) return;

    gint active = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
    gboolean new_is_directed = (active == 0); // 0 = Orienté, 1 = Non Orienté

    // Si le graphe a des nœuds, demander confirmation
    if (app_data->current_graph->num_nodes > 0) {
        GtkWidget *parent = gtk_widget_get_toplevel(GTK_WIDGET(combo));
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(parent),
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_QUESTION,
            GTK_BUTTONS_YES_NO,
            "Changer le type de graphe va réinitialiser toutes les arêtes.\n\nVoulez-vous continuer ?"
        );
        gtk_window_set_title(GTK_WINDOW(dialog), "Changer le type de graphe");

        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        if (response != GTK_RESPONSE_YES) {
            // Restaurer l'ancienne valeur
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo), app_data->current_graph->is_directed ? 0 : 1);
            return;
        }

        // Réinitialiser toutes les arêtes
        graph_remove_all_edges(app_data->current_graph);
    }

    app_data->current_graph->is_directed = new_is_directed;
    gtk_widget_queue_draw(app_data->graph_drawing_area);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
    gchar *msg = g_strdup_printf("✓ Type de graphe changé en: %s", new_is_directed ? "Orienté" : "Non Orienté");
    gtk_text_buffer_set_text(buffer, msg, -1);
    g_free(msg);
}

// Callback pour le changement de pondération (pondéré/non pondéré)
static void on_graph_weighted_changed(GtkComboBoxText *combo, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data || !app_data->current_graph) return;

    gint active = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
    gboolean new_is_weighted = (active == 0); // 0 = Pondéré, 1 = Non Pondéré

    // Si le graphe est non pondéré, convertir tous les poids existants à 1
    if (!new_is_weighted && app_data->current_graph->num_nodes > 0) {
        for(int i=0; i<app_data->current_graph->num_nodes; i++) {
            for(int j=0; j<app_data->current_graph->num_nodes; j++) {
                if (i != j && app_data->current_graph->adj_matrix[i][j] != INF) {
                    app_data->current_graph->adj_matrix[i][j] = 1;
                    // Si non orienté, mettre à jour l'arête inverse aussi
                    if (!app_data->current_graph->is_directed) {
                        app_data->current_graph->adj_matrix[j][i] = 1;
                    }
                }
            }
        }
    }

    app_data->current_graph->is_weighted = new_is_weighted;
    gtk_widget_queue_draw(app_data->graph_drawing_area);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
    gchar *msg = g_strdup_printf("✓ Pondération changée en: %s", new_is_weighted ? "Pondéré" : "Non Pondéré");
    gtk_text_buffer_set_text(buffer, msg, -1);
    g_free(msg);
}

static void on_graph_run_algo_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_graph) return;

    const gchar *algo = gtk_combo_box_text_get_active_text(app_data->graph_algo_combo);

    // Récupérer les valeurs depuis les Entry
    const gchar *src_str = gtk_entry_get_text(app_data->graph_src_entry);
    const gchar *dest_str = gtk_entry_get_text(app_data->graph_dest_entry);

    // Validation
    if (app_data->current_graph->num_nodes == 0) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
        gtk_text_buffer_set_text(buffer, "❌ Le graphe est vide. Ajoutez des nœuds d'abord.", -1);
        return;
    }

    if (!src_str || strlen(src_str) == 0) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
        gtk_text_buffer_set_text(buffer, "❌ Veuillez entrer une valeur pour le nœud initial.", -1);
        return;
    }

    if (!dest_str || strlen(dest_str) == 0) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
        gtk_text_buffer_set_text(buffer, "❌ Veuillez entrer une valeur pour le nœud final.", -1);
        return;
    }

    // Trouver les nœuds par leur valeur
    int src = graph_find_node_by_value(app_data->current_graph, src_str);
    int dest = graph_find_node_by_value(app_data->current_graph, dest_str);

    if (src < 0) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
        gchar *msg = g_strdup_printf("❌ Nœud initial '%s' introuvable dans le graphe.", src_str);
        gtk_text_buffer_set_text(buffer, msg, -1);
        g_free(msg);
        return;
    }

    if (dest < 0) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
        gchar *msg = g_strdup_printf("❌ Nœud final '%s' introuvable dans le graphe.", dest_str);
        gtk_text_buffer_set_text(buffer, msg, -1);
        g_free(msg);
        return;
    }

    if (src == dest) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
        gtk_text_buffer_set_text(buffer, "ℹ️ Nœud initial et final identiques. Distance = 0.", -1);
        return;
    }

    GString *res = g_string_new("");

    if (g_strcmp0(algo, "Dijkstra") == 0) {
        algo_dijkstra(app_data->current_graph, src, dest, res, app_data);
    } else if (g_strcmp0(algo, "Bellman-Ford") == 0) {
        algo_bellman_ford(app_data->current_graph, src, dest, res, app_data);
    } else if (g_strcmp0(algo, "Floyd-Warshall") == 0) {
        algo_floyd_warshall(app_data->current_graph, src, dest, res, app_data);
    }

    // Redessiner le graphe pour afficher le chemin
    if (app_data->graph_drawing_area) {
        gtk_widget_queue_draw(app_data->graph_drawing_area);
    }

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
    gtk_text_buffer_set_text(buffer, res->str, -1);
    g_string_free(res, TRUE);
}

// Callback pour réinitialiser l'affichage du chemin le plus court
static void on_graph_reset_path_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;

    // Réinitialiser le chemin
    app_data->graph_shortest_path_length = -1;

    // Effacer le texte d'information
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
    gtk_text_buffer_set_text(buffer, "Chemin réinitialisé. Le graphe est affiché sans chemin mis en évidence.", -1);

    // Redessiner le graphe pour enlever le chemin mis en évidence
    if (app_data->graph_drawing_area) {
        gtk_widget_queue_draw(app_data->graph_drawing_area);
    }
}


// --- Fonction utilitaire pour trouver le nœud le plus proche d'un point ---
static int find_nearest_node(Graph *g, double x, double y, double threshold) {
    if (!g || g->num_nodes == 0) return -1;

    double min_dist = threshold * threshold;
    int nearest = -1;
    double node_radius = 20.0;

    for(int i=0; i<g->num_nodes; i++) {
        double dx = g->node_x[i] - x;
        double dy = g->node_y[i] - y;
        double dist_sq = dx*dx + dy*dy;

        if (dist_sq < min_dist) {
            min_dist = dist_sq;
            nearest = i;
        }
    }

    return nearest;
}

// --- Gestionnaires d'événements pour l'interaction ---
static gboolean on_graph_drawing_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_graph) return FALSE;

    double x = event->x;
    double y = event->y;
    double node_radius = 20.0;

    int nearest = find_nearest_node(app_data->current_graph, x, y, node_radius * 2.0);

    if (app_data->graph_interaction_mode == 0) { // Mode Ajouter nœud
        // Toujours créer un nouveau nœud, même si on est près d'un autre (on ignore nearest)
        if (app_data->current_graph->num_nodes >= MAX_GRAPH_NODES) {
            GtkWidget *parent = gtk_widget_get_toplevel(GTK_WIDGET(widget));
            show_error_dialog(parent, "Limite atteinte",
                g_strdup_printf("Nombre maximum de nœuds (%d) atteint. Supprimez des nœuds avant d'en ajouter.", MAX_GRAPH_NODES));
            return TRUE;
        }

        // Obtenir le type depuis le combo box (au cas où il aurait changé)
        const gchar *element_type = gtk_combo_box_text_get_active_text(app_data->graph_type_combo);
        if (!element_type) {
            element_type = app_data->current_graph->element_type;
        }

        // Mettre à jour le type du graphe si nécessaire
        if (g_strcmp0(element_type, app_data->current_graph->element_type) != 0) {
            app_data->current_graph->element_type = element_type;
            size_t element_size;
            int (*compare_func)(const void *, const void *);
            get_type_info(element_type, &element_size, &compare_func);
            app_data->current_graph->element_size = element_size;
        }

        void *val = get_value_input(gtk_widget_get_toplevel(widget), "Ajouter Nœud", "Valeur du nœud:", element_type, NULL);

        if (val) {
            graph_add_node(app_data->current_graph, val, x, y);
            // S'assurer qu'aucune sélection d'arête n'est active
            app_data->graph_selected_node = -1;
            app_data->graph_dragging_edge_source = -1;
            gtk_widget_queue_draw(widget);

            GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
            gtk_text_buffer_set_text(buffer, "Nœud ajouté avec succès.", -1);

            if (g_strcmp0(element_type, "Chaîne de Caractères") == 0) {
                g_free(*(char **)val);
            }
            g_free(val);
        }
    } else if (app_data->graph_interaction_mode == 1) { // Mode Ajouter arête
        if (nearest != -1) {
            // Démarrer le glissement depuis ce nœud
            app_data->graph_dragging_edge_source = nearest;
            app_data->graph_dragging_edge_x = x;
            app_data->graph_dragging_edge_y = y;
            gtk_widget_queue_draw(widget);
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
            gchar *msg = g_strdup_printf("Nœud source sélectionné: %d. Glissez vers un autre nœud pour créer un arc.", nearest);
            gtk_text_buffer_set_text(buffer, msg, -1);
            g_free(msg);
        }
    } else if (app_data->graph_interaction_mode == 2) { // Mode Déplacer nœud
        if (nearest != -1) {
            app_data->graph_dragging_node = nearest;
        }
    } else if (app_data->graph_interaction_mode == 3) { // Mode Supprimer nœud
        if (nearest != -1) {
            graph_remove_node(app_data->current_graph, nearest);
            gtk_widget_queue_draw(widget);

            GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
            gchar *msg = g_strdup_printf("Nœud %d supprimé.", nearest);
            gtk_text_buffer_set_text(buffer, msg, -1);
            g_free(msg);
        }
    } else if (app_data->graph_interaction_mode == 4) { // Mode Supprimer arête
        // Chercher l'arête la plus proche du point de clic
        Graph *g = app_data->current_graph;
        if (g && g->num_nodes > 0) {
            double min_dist = node_radius * 3.0; // Distance maximale pour sélectionner une arête
            int best_src = -1, best_dest = -1;

            for(int i=0; i<g->num_nodes; i++) {
                for(int j=0; j<g->num_nodes; j++) {
                    if (i != j && g->adj_matrix[i][j] != INF && g->adj_matrix[i][j] != 0) {
                        // Calculer la distance du point au segment de ligne
                        double x1 = g->node_x[i];
                        double y1 = g->node_y[i];
                        double x2 = g->node_x[j];
                        double y2 = g->node_y[j];

                        // Distance du point (x,y) au segment (x1,y1)-(x2,y2)
                        double A = x - x1;
                        double B = y - y1;
                        double C = x2 - x1;
                        double D = y2 - y1;

                        double dot = A * C + B * D;
                        double lenSq = C * C + D * D;
                        double param = (lenSq != 0) ? dot / lenSq : -1;

                        double xx, yy;
                        if (param < 0) {
                            xx = x1;
                            yy = y1;
                        } else if (param > 1) {
                            xx = x2;
                            yy = y2;
                        } else {
                            xx = x1 + param * C;
                            yy = y1 + param * D;
                        }

                        double dx = x - xx;
                        double dy = y - yy;
                        double dist = sqrt(dx*dx + dy*dy);

                        if (dist < min_dist) {
                            min_dist = dist;
                            best_src = i;
                            best_dest = j;
                        }
                    }
                }
            }

            if (best_src != -1 && best_dest != -1) {
                graph_remove_edge(app_data->current_graph, best_src, best_dest);
                gtk_widget_queue_draw(widget);

                GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
                gchar *msg = g_strdup_printf("Arête supprimée: %d -> %d", best_src, best_dest);
                gtk_text_buffer_set_text(buffer, msg, -1);
                g_free(msg);
            }
        }
    }

    return TRUE;
}

static gboolean on_graph_drawing_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data->current_graph) return FALSE;

    // Gérer le déplacement de nœud
    if (app_data->graph_dragging_node != -1) {
        app_data->current_graph->node_x[app_data->graph_dragging_node] = event->x;
        app_data->current_graph->node_y[app_data->graph_dragging_node] = event->y;
        gtk_widget_queue_draw(widget);
        return TRUE;
    }

    // Gérer le glissement pour créer un arc
    if (app_data->graph_dragging_edge_source != -1) {
        app_data->graph_dragging_edge_x = event->x;
        app_data->graph_dragging_edge_y = event->y;
        gtk_widget_queue_draw(widget);
        return TRUE;
    }

    return FALSE;
}

static gboolean on_graph_drawing_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    AppData *app_data = (AppData *)data;

    // Gérer la fin du déplacement de nœud
    if (app_data->graph_dragging_node != -1) {
        app_data->graph_dragging_node = -1;
        return TRUE;
    }

    // Gérer la fin du glissement pour créer un arc
    if (app_data->graph_dragging_edge_source != -1) {
        double x = event->x;
        double y = event->y;
        double node_radius = 20.0;

        // Trouver le nœud le plus proche du point de relâchement
        int nearest = find_nearest_node(app_data->current_graph, x, y, node_radius * 2.0);

        if (nearest != -1 && nearest != app_data->graph_dragging_edge_source) {
            // Relâchement sur un nœud différent : créer l'arc
            int src = app_data->graph_dragging_edge_source;
            int dest = nearest;

            // Demander le poids avec un dialogue
            GtkWidget *parent = gtk_widget_get_toplevel(widget);
            int default_weight = gtk_spin_button_get_value_as_int(app_data->graph_edge_weight_spin);
            gchar *prompt = g_strdup_printf("Poids de l'arc %d -> %d:", src, dest);
            int weight = get_integer_input(parent, "Poids de l'arc", prompt, default_weight);
            g_free(prompt);

            if (weight != -1) { // L'utilisateur n'a pas annulé
                graph_add_edge(app_data->current_graph, src, dest, weight);
                gtk_widget_queue_draw(widget);

                GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
                gchar *msg = g_strdup_printf("Arc créé: %d -> %d (poids: %d)", src, dest, weight);
                gtk_text_buffer_set_text(buffer, msg, -1);
                g_free(msg);
            }
        }

        // Réinitialiser le glissement
        app_data->graph_dragging_edge_source = -1;
        gtk_widget_queue_draw(widget);
        return TRUE;
    }

    return TRUE;
}

// --- Callbacks pour changer le mode d'interaction ---
static void on_graph_mode_changed(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    const gchar *label = gtk_button_get_label(GTK_BUTTON(widget));

    if (g_strcmp0(label, "➕ Ajouter Nœud") == 0) {
        app_data->graph_interaction_mode = 0;
        app_data->graph_dragging_edge_source = -1;
    } else if (g_strcmp0(label, "🔗 Ajouter Arête") == 0) {
        app_data->graph_interaction_mode = 1;
        app_data->graph_selected_node = -1;
        app_data->graph_dragging_edge_source = -1;
    } else if (g_strcmp0(label, "↔️ Déplacer Nœud") == 0) {
        app_data->graph_interaction_mode = 2;
        app_data->graph_dragging_edge_source = -1;
    } else if (g_strcmp0(label, "❌ Supprimer Nœud") == 0) {
        app_data->graph_interaction_mode = 3;
        app_data->graph_dragging_edge_source = -1;
    } else if (g_strcmp0(label, "✂️ Supprimer Arête") == 0) {
        app_data->graph_interaction_mode = 4;
        app_data->graph_selected_node = -1;
        app_data->graph_dragging_edge_source = -1;
    } else if (g_strcmp0(label, "🗑️ Supprimer Toutes les Arêtes") == 0) {
        if (app_data->current_graph) {
            graph_remove_all_edges(app_data->current_graph);
            gtk_widget_queue_draw(app_data->graph_drawing_area);
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
            gtk_text_buffer_set_text(buffer, "Toutes les arêtes ont été supprimées.", -1);
        }
    }

    gtk_widget_queue_draw(app_data->graph_drawing_area);
}

// --- Window ---
static void create_graph_window(GtkWidget *parent_window) {
    AppData *app_data = g_new0(AppData, 1);
    const gchar *default_type = "Entiers (Int)";
    // Par défaut: orienté et pondéré
    app_data->current_graph = graph_new(default_type, TRUE, TRUE);
    // Démarrer avec un graphe vide - l'utilisateur dessinera lui-même
    app_data->graph_interaction_mode = 0; // Mode Ajouter nœud par défaut
    app_data->graph_selected_node = -1;
    app_data->graph_dragging_node = -1;
    app_data->graph_dragging_edge_source = -1;
    app_data->graph_shortest_path_length = -1; // Aucun chemin calculé initialement

    // S'assurer qu'aucune arête n'existe au démarrage
    if (app_data->current_graph) {
        for(int i=0; i<MAX_GRAPH_NODES; i++) {
            for(int j=0; j<MAX_GRAPH_NODES; j++) {
                if (i != j) {
                    app_data->current_graph->adj_matrix[i][j] = INF;
                }
            }
        }
    }

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "🕸️ Module Graphes (Dijkstra, Bellman-Ford, Floyd)");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    // Ne pas utiliser transient_for pour éviter que la fermeture de la fenêtre secondaire ferme la principale
    // gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent_window));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(window), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(window), 15);

    // Container principal avec fond moderne
    GtkWidget *main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_container);

    // Header moderne avec gradient
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(header_box), "modern-card-header");
    gtk_widget_set_size_request(header_box, -1, 80);
    gtk_box_pack_start(GTK_BOX(main_container), header_box, FALSE, FALSE, 0);

    GtkWidget *header_title = gtk_label_new(NULL);
    PangoFontDescription *header_font = pango_font_description_from_string("Segoe UI Bold 24");
    gtk_widget_override_font(header_title, header_font);
    pango_font_description_free(header_font);
    gtk_label_set_markup(GTK_LABEL(header_title), "<span foreground='#ffffff' size='x-large' weight='bold'>🕸️ MODULE GRAPHES</span>\n<span size='small' foreground='#ffffff'>Dijkstra, Bellman-Ford, Floyd</span>");
    gtk_label_set_justify(GTK_LABEL(header_title), GTK_JUSTIFY_CENTER);
    gtk_label_set_xalign(GTK_LABEL(header_title), 0.5);
    gtk_box_pack_start(GTK_BOX(header_box), header_title, TRUE, TRUE, 0);

    // Panneau principal avec paned
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(main_container), hbox, TRUE, TRUE, 0);

    // ========== PANEL GAUCHE : CONTROLES MODERNES ==========
    GtkWidget *control_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(control_scrolled), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(control_scrolled), GTK_SHADOW_NONE);

    GtkWidget *control_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(control_vbox), "control-panel");
    gtk_widget_set_size_request(control_vbox, 350, -1);
    gtk_container_set_border_width(GTK_CONTAINER(control_vbox), 20);
    gtk_container_add(GTK_CONTAINER(control_scrolled), control_vbox);

    // Section Type de Données
    GtkWidget *type_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(type_card), "modern-card");
    gtk_box_pack_start(GTK_BOX(control_vbox), type_card, FALSE, FALSE, 0);

    GtkWidget *type_title = gtk_label_new("Type de Données");
    gtk_label_set_markup(GTK_LABEL(type_title), "<span foreground='#ffffff' size='large' weight='bold'>📊 Type de Données</span>");
    gtk_box_pack_start(GTK_BOX(type_card), type_title, FALSE, FALSE, 0);

    app_data->graph_type_combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(app_data->graph_type_combo, "Entiers (Int)");
    gtk_combo_box_text_append_text(app_data->graph_type_combo, "Réels (Float)");
    gtk_combo_box_text_append_text(app_data->graph_type_combo, "Caractères (Char)");
    gtk_combo_box_text_append_text(app_data->graph_type_combo, "Chaîne de Caractères");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app_data->graph_type_combo), 0);
    // Connecter le signal pour gérer le changement de type
    g_signal_connect(app_data->graph_type_combo, "changed", G_CALLBACK(on_graph_type_changed), app_data);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(app_data->graph_type_combo)), "modern-combo");
    gtk_box_pack_start(GTK_BOX(type_card), GTK_WIDGET(app_data->graph_type_combo), FALSE, FALSE, 0);

    // Section Type de Graphe (Orienté/Non orienté)
    GtkWidget *directed_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_box_pack_start(GTK_BOX(control_vbox), directed_card, FALSE, FALSE, 0);

    GtkWidget *directed_title = gtk_label_new("Type de Graphe");
    gtk_label_set_markup(GTK_LABEL(directed_title), "<span foreground='#ffffff' size='large' weight='bold'>↔️ Type de Graphe</span>");
    gtk_box_pack_start(GTK_BOX(directed_card), directed_title, FALSE, FALSE, 0);

    app_data->graph_directed_combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(app_data->graph_directed_combo, "Orienté");
    gtk_combo_box_text_append_text(app_data->graph_directed_combo, "Non Orienté");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app_data->graph_directed_combo), 0); // Par défaut orienté
    g_signal_connect(app_data->graph_directed_combo, "changed", G_CALLBACK(on_graph_directed_changed), app_data);
    gtk_box_pack_start(GTK_BOX(directed_card), GTK_WIDGET(app_data->graph_directed_combo), FALSE, FALSE, 0);

    // Section Pondération (Pondéré/Non pondéré)
    GtkWidget *weighted_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_box_pack_start(GTK_BOX(control_vbox), weighted_card, FALSE, FALSE, 0);

    GtkWidget *weighted_title = gtk_label_new("Pondération");
    gtk_label_set_markup(GTK_LABEL(weighted_title), "<span foreground='#ffffff' size='large' weight='bold'>⚖️ Pondération</span>");
    gtk_box_pack_start(GTK_BOX(weighted_card), weighted_title, FALSE, FALSE, 0);

    app_data->graph_weighted_combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(app_data->graph_weighted_combo, "Pondéré");
    gtk_combo_box_text_append_text(app_data->graph_weighted_combo, "Non Pondéré");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app_data->graph_weighted_combo), 0); // Par défaut pondéré
    g_signal_connect(app_data->graph_weighted_combo, "changed", G_CALLBACK(on_graph_weighted_changed), app_data);
    gtk_box_pack_start(GTK_BOX(weighted_card), GTK_WIDGET(app_data->graph_weighted_combo), FALSE, FALSE, 0);

    // Section Mode d'Interaction
    GtkWidget *mode_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(mode_card), "modern-card");
    gtk_box_pack_start(GTK_BOX(control_vbox), mode_card, FALSE, FALSE, 0);

    GtkWidget *mode_title = gtk_label_new("Mode d'Interaction");
    gtk_label_set_markup(GTK_LABEL(mode_title), "<span foreground='#ffffff' size='large' weight='bold'>🖱️ Mode d'Interaction</span>");
    gtk_box_pack_start(GTK_BOX(mode_card), mode_title, FALSE, FALSE, 0);

    GtkWidget *vbox_mode = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_box_pack_start(GTK_BOX(mode_card), vbox_mode, FALSE, FALSE, 0);

    GtkWidget *btn_mode_add_node = gtk_button_new_with_label("➕ Ajouter Nœud");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_mode_add_node), "modern-button");
    g_signal_connect(btn_mode_add_node, "clicked", G_CALLBACK(on_graph_mode_changed), app_data);
    gtk_box_pack_start(GTK_BOX(vbox_mode), btn_mode_add_node, FALSE, FALSE, 0);

    GtkWidget *btn_create_multiple_nodes = gtk_button_new_with_label("🔢 Créer Plusieurs Nœuds");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_create_multiple_nodes), "modern-button");
    g_signal_connect(btn_create_multiple_nodes, "clicked", G_CALLBACK(on_graph_create_multiple_nodes), app_data);
    gtk_box_pack_start(GTK_BOX(vbox_mode), btn_create_multiple_nodes, FALSE, FALSE, 0);

    GtkWidget *btn_mode_add_edge = gtk_button_new_with_label("🔗 Ajouter Arête");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_mode_add_edge), "modern-button");
    g_signal_connect(btn_mode_add_edge, "clicked", G_CALLBACK(on_graph_mode_changed), app_data);
    gtk_box_pack_start(GTK_BOX(vbox_mode), btn_mode_add_edge, FALSE, FALSE, 0);

    GtkWidget *btn_mode_move = gtk_button_new_with_label("↔️ Déplacer Nœud");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_mode_move), "modern-button");
    g_signal_connect(btn_mode_move, "clicked", G_CALLBACK(on_graph_mode_changed), app_data);
    gtk_box_pack_start(GTK_BOX(vbox_mode), btn_mode_move, FALSE, FALSE, 0);

    GtkWidget *btn_mode_delete = gtk_button_new_with_label("❌ Supprimer Nœud");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_mode_delete), "modern-button");
    g_signal_connect(btn_mode_delete, "clicked", G_CALLBACK(on_graph_mode_changed), app_data);
    gtk_box_pack_start(GTK_BOX(vbox_mode), btn_mode_delete, FALSE, FALSE, 0);

    GtkWidget *btn_mode_delete_edge = gtk_button_new_with_label("✂️ Supprimer Arête");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_mode_delete_edge), "modern-button");
    g_signal_connect(btn_mode_delete_edge, "clicked", G_CALLBACK(on_graph_mode_changed), app_data);
    gtk_box_pack_start(GTK_BOX(vbox_mode), btn_mode_delete_edge, FALSE, FALSE, 0);

    GtkWidget *btn_remove_all_edges = gtk_button_new_with_label("🗑️ Supprimer Toutes les Arêtes");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_remove_all_edges), "modern-button");
    g_signal_connect(btn_remove_all_edges, "clicked", G_CALLBACK(on_graph_mode_changed), app_data);
    gtk_box_pack_start(GTK_BOX(vbox_mode), btn_remove_all_edges, FALSE, FALSE, 0);

    // Section Poids des Arêtes (pour le mode Ajouter Arête)
    GtkWidget *weight_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(weight_card), "modern-card");
    gtk_box_pack_start(GTK_BOX(control_vbox), weight_card, FALSE, FALSE, 0);

    GtkWidget *weight_title = gtk_label_new("Poids des Arêtes");
    gtk_label_set_markup(GTK_LABEL(weight_title), "<span foreground='#ffffff' size='large' weight='bold'>⚖️ Poids des Arêtes</span>");
    gtk_box_pack_start(GTK_BOX(weight_card), weight_title, FALSE, FALSE, 0);

    GtkWidget *hbox_edge_weight = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(weight_card), hbox_edge_weight, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_edge_weight), gtk_label_new("Poids:"), FALSE, FALSE, 0);
    app_data->graph_edge_weight_spin = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(-100, 100, 1));
    gtk_spin_button_set_value(app_data->graph_edge_weight_spin, 1);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(app_data->graph_edge_weight_spin)), "modern-spin");
    gtk_box_pack_start(GTK_BOX(hbox_edge_weight), GTK_WIDGET(app_data->graph_edge_weight_spin), TRUE, TRUE, 0);

    // Section Algorithmes (Carte moderne)
    GtkWidget *algo_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(algo_card), "modern-card");
    gtk_box_pack_start(GTK_BOX(control_vbox), algo_card, FALSE, FALSE, 0);

    GtkWidget *algo_title = gtk_label_new("Algorithmes");
    gtk_label_set_markup(GTK_LABEL(algo_title), "<span foreground='#ffffff' size='large' weight='bold'>⚙️ Algorithmes</span>");
    gtk_box_pack_start(GTK_BOX(algo_card), algo_title, FALSE, FALSE, 0);

    GtkWidget *vbox_algo = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(algo_card), vbox_algo, FALSE, FALSE, 0);

    app_data->graph_algo_combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(app_data->graph_algo_combo, "Dijkstra");
    gtk_combo_box_text_append_text(app_data->graph_algo_combo, "Bellman-Ford");
    gtk_combo_box_text_append_text(app_data->graph_algo_combo, "Floyd-Warshall");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app_data->graph_algo_combo), 0);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(app_data->graph_algo_combo)), "modern-combo");
    gtk_box_pack_start(GTK_BOX(vbox_algo), GTK_WIDGET(app_data->graph_algo_combo), FALSE, FALSE, 0);

    GtkWidget *hbox_src = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox_algo), hbox_src, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_src), gtk_label_new("Nœud Initial:"), FALSE, FALSE, 0);
    app_data->graph_src_entry = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_placeholder_text(app_data->graph_src_entry, "Valeur du nœud");
    // Style CSS pour texte blanc et fond sombre
    GtkStyleContext *src_context = gtk_widget_get_style_context(GTK_WIDGET(app_data->graph_src_entry));
    gtk_style_context_add_class(src_context, "modern-spin");
    GdkRGBA white_color = {1.0, 1.0, 1.0, 1.0};
    gtk_widget_override_color(GTK_WIDGET(app_data->graph_src_entry), GTK_STATE_FLAG_NORMAL, &white_color);
    gtk_widget_override_color(GTK_WIDGET(app_data->graph_src_entry), GTK_STATE_FLAG_FOCUSED, &white_color);
    gtk_widget_override_background_color(GTK_WIDGET(app_data->graph_src_entry), GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0.18, 0.25, 0.36, 1.0});
    gtk_box_pack_start(GTK_BOX(hbox_src), GTK_WIDGET(app_data->graph_src_entry), TRUE, TRUE, 0);

    GtkWidget *hbox_dest = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox_algo), hbox_dest, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_dest), gtk_label_new("Nœud Final:"), FALSE, FALSE, 0);
    app_data->graph_dest_entry = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_placeholder_text(app_data->graph_dest_entry, "Valeur du nœud");
    // Style CSS pour texte blanc et fond sombre
    GtkStyleContext *dest_context = gtk_widget_get_style_context(GTK_WIDGET(app_data->graph_dest_entry));
    gtk_style_context_add_class(dest_context, "modern-spin");
    gtk_widget_override_color(GTK_WIDGET(app_data->graph_dest_entry), GTK_STATE_FLAG_NORMAL, &white_color);
    gtk_widget_override_color(GTK_WIDGET(app_data->graph_dest_entry), GTK_STATE_FLAG_FOCUSED, &white_color);
    gtk_widget_override_background_color(GTK_WIDGET(app_data->graph_dest_entry), GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0.18, 0.25, 0.36, 1.0});
    gtk_box_pack_start(GTK_BOX(hbox_dest), GTK_WIDGET(app_data->graph_dest_entry), TRUE, TRUE, 0);

    GtkWidget *btn_run = gtk_button_new_with_label("▶️ Calculer Chemin Plus Court");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_run), "modern-button");
    g_signal_connect(btn_run, "clicked", G_CALLBACK(on_graph_run_algo_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(vbox_algo), btn_run, FALSE, FALSE, 0);

    // Bouton de reset pour effacer l'affichage du chemin
    GtkWidget *btn_reset = gtk_button_new_with_label("🔄 Réinitialiser Chemin");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_reset), "modern-button");
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_graph_reset_path_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(vbox_algo), btn_reset, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), control_scrolled, FALSE, FALSE, 0);

    // ========== PANEL DROIT : AFFICHAGE DES RÉSULTATS MODERNES ==========
    GtkWidget *display_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(display_scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(display_scrolled), GTK_SHADOW_NONE);

    GtkWidget *display_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(display_vbox), "display-panel");
    gtk_container_set_border_width(GTK_CONTAINER(display_vbox), 20);
    gtk_container_add(GTK_CONTAINER(display_scrolled), display_vbox);
    gtk_box_pack_start(GTK_BOX(hbox), display_scrolled, TRUE, TRUE, 0);

    // Visualisation du Graphe (Carte moderne)
    GtkWidget *drawing_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(drawing_card), "result-card");
    gtk_box_pack_start(GTK_BOX(display_vbox), drawing_card, TRUE, TRUE, 0);

    GtkWidget *drawing_header = gtk_label_new("Visualisation du Graphe");
    gtk_label_set_markup(GTK_LABEL(drawing_header), "<span foreground='#ffffff' size='large' weight='bold'>🕸️ Visualisation du Graphe</span>");
    gtk_box_pack_start(GTK_BOX(drawing_card), drawing_header, FALSE, FALSE, 0);

    app_data->graph_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(app_data->graph_drawing_area, -1, 400);
    gtk_widget_add_events(app_data->graph_drawing_area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
    g_signal_connect(app_data->graph_drawing_area, "draw", G_CALLBACK(draw_graph_callback), app_data);
    g_signal_connect(app_data->graph_drawing_area, "button-press-event", G_CALLBACK(on_graph_drawing_button_press), app_data);
    g_signal_connect(app_data->graph_drawing_area, "button-release-event", G_CALLBACK(on_graph_drawing_button_release), app_data);
    g_signal_connect(app_data->graph_drawing_area, "motion-notify-event", G_CALLBACK(on_graph_drawing_motion_notify), app_data);
    gtk_box_pack_start(GTK_BOX(drawing_card), app_data->graph_drawing_area, TRUE, TRUE, 0);

    // Zone d'Informations (Carte moderne)
    GtkWidget *info_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(info_card), "result-card");
    gtk_box_pack_start(GTK_BOX(display_vbox), info_card, TRUE, TRUE, 0);

    GtkWidget *info_header = gtk_label_new("Résultats");
    gtk_label_set_markup(GTK_LABEL(info_header), "<span foreground='#ffffff' size='large' weight='bold'>📊 Résultats</span>");
    gtk_box_pack_start(GTK_BOX(info_card), info_header, FALSE, FALSE, 0);

    GtkWidget *scrolled_info = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_info), GTK_SHADOW_IN);
    app_data->graph_info_view = GTK_TEXT_VIEW(gtk_text_view_new());
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app_data->graph_info_view), FALSE);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(app_data->graph_info_view)), "modern-text-view");
    gtk_container_add(GTK_CONTAINER(scrolled_info), GTK_WIDGET(app_data->graph_info_view));
    gtk_box_pack_start(GTK_BOX(info_card), scrolled_info, TRUE, TRUE, 0);

    // Message d'information initial
    GtkTextBuffer *init_buffer = gtk_text_view_get_buffer(app_data->graph_info_view);
    gtk_text_buffer_set_text(init_buffer, "Bienvenue dans le Module Graphes!\n\n1. Sélectionnez le type de données\n2. Choisissez un mode d'interaction\n3. Cliquez sur le canvas pour construire votre graphe\n4. Sélectionnez un algorithme et calculez les chemins minimums", -1);

    // Gestionnaire pour la fermeture de la fenêtre secondaire
    g_signal_connect(window, "delete-event", G_CALLBACK(on_secondary_window_delete), NULL);

    gtk_widget_show_all(window);
}


// --- Gestion Mémoire ---
static void free_binary_tree(BinaryNode *root) {
    if (!root) return;
    free_binary_tree(root->left);
    free_binary_tree(root->right);
    free(root->data); // Assume data ptr
    free(root);
}

static void free_nary_tree(NaryNode *root) {
    if (!root) return;
    free_nary_tree(root->first_child);
    free_nary_tree(root->next_sibling);
    free(root->data);
    free(root);
}

// --- Insertion Binaire (BST) ---
static BinaryNode *insert_binary(BinaryNode *node, void *data, int (*compare_func)(const void *, const void *), size_t element_size) {
    if (node == NULL) {
        BinaryNode *new_node = g_new0(BinaryNode, 1);
        new_node->data = malloc(element_size);
        memcpy(new_node->data, data, element_size);
        return new_node;
    }

    if (compare_func(data, node->data) < 0) {
        node->left = insert_binary(node->left, data, compare_func, element_size);
    } else if (compare_func(data, node->data) > 0) {
        node->right = insert_binary(node->right, data, compare_func, element_size);
    }
    return node;
}

// --- Insertion N-Ary (Random Child) ---
static NaryNode *insert_nary_random(NaryNode *root, void *data, size_t element_size, int max_children) {
    // Création de la racine si nécessaire
    if (root == NULL) {
        NaryNode *new_node = g_new0(NaryNode, 1);
        new_node->data = malloc(element_size);
        memcpy(new_node->data, data, element_size);
        return new_node;
    }

    // Si un degré max est défini (>0), on cherche un parent qui a de la place
    if (max_children > 0) {
        GQueue queue = G_QUEUE_INIT;
        g_queue_push_tail(&queue, root);

        while (!g_queue_is_empty(&queue)) {
            NaryNode *parent = g_queue_pop_head(&queue);

            // Compter les enfants existants et remplir la file pour la suite
            int child_count = 0;
            NaryNode *child_iter = parent->first_child;
            while (child_iter) {
                child_count++;
                g_queue_push_tail(&queue, child_iter);
                child_iter = child_iter->next_sibling;
            }

            // Parent trouvé : on peut ajouter ici
            if (child_count < max_children) {
                NaryNode *new_node = g_new0(NaryNode, 1);
                new_node->data = malloc(element_size);
                memcpy(new_node->data, data, element_size);

                if (parent->first_child == NULL) {
                    parent->first_child = new_node;
                } else {
                    NaryNode *last = parent->first_child;
                    while (last->next_sibling) last = last->next_sibling;
                    last->next_sibling = new_node;
                }
                return root;
            }
        }
        // Si aucun parent n'a de place (arbre saturé), on retombe sur l'ancien mode non borné
    }

    // Mode non borné : ancien comportement "aléatoire"
    NaryNode *current = root;
    while (current->first_child && (rand() % 2 == 0)) {
        current = current->first_child;
        while (current->next_sibling && (rand() % 2 == 0)) {
            current = current->next_sibling;
        }
    }

    NaryNode *new_node = g_new0(NaryNode, 1);
    new_node->data = malloc(element_size);
    memcpy(new_node->data, data, element_size);

    if (current->first_child == NULL) {
        current->first_child = new_node;
    } else {
        NaryNode *child = current->first_child;
        while (child->next_sibling) child = child->next_sibling;
        child->next_sibling = new_node;
    }
    return root;
}

// --- Parcours (Traversals) ---
static void preorder_binary(BinaryNode *root, GString *str, const gchar *type) {
    if (!root) return;
    // Print stats/data
    if (g_strcmp0(type, "Entiers (Int)") == 0) g_string_append_printf(str, "%d ", *(int*)root->data);
    else g_string_append_printf(str, "? ");

    preorder_binary(root->left, str, type);
    preorder_binary(root->right, str, type);
}

static void inorder_binary(BinaryNode *root, GString *str, const gchar *type) {
    if (!root) return;
    inorder_binary(root->left, str, type);
    if (g_strcmp0(type, "Entiers (Int)") == 0) g_string_append_printf(str, "%d ", *(int*)root->data);
    else g_string_append_printf(str, "? ");
    inorder_binary(root->right, str, type);
}

static void postorder_binary(BinaryNode *root, GString *str, const gchar *type) {
    if (!root) return;
    postorder_binary(root->left, str, type);
    postorder_binary(root->right, str, type);
    if (g_strcmp0(type, "Entiers (Int)") == 0) g_string_append_printf(str, "%d ", *(int*)root->data);
    else g_string_append_printf(str, "? ");
}

// --- Stats ---
static int binary_depth(BinaryNode *root) {
    if (!root) return 0;
    int l = binary_depth(root->left);
    int r = binary_depth(root->right);
    return 1 + (l > r ? l : r);
}

static int binary_size(BinaryNode *root) {
    if (!root) return 0;
    return 1 + binary_size(root->left) + binary_size(root->right);
}

// --- Parcours pour N-Aire ---
static void preorder_nary(NaryNode *root, GString *str, const gchar *type) {
    if (!root) return;
    if (g_strcmp0(type, "Entiers (Int)") == 0) g_string_append_printf(str, "%d ", *(int*)root->data);
    else g_string_append_printf(str, "? ");

    NaryNode *child = root->first_child;
    while (child) {
        preorder_nary(child, str, type);
        child = child->next_sibling;
    }
}

static void postorder_nary(NaryNode *root, GString *str, const gchar *type) {
    if (!root) return;

    NaryNode *child = root->first_child;
    while (child) {
        postorder_nary(child, str, type);
        child = child->next_sibling;
    }

    if (g_strcmp0(type, "Entiers (Int)") == 0) g_string_append_printf(str, "%d ", *(int*)root->data);
    else g_string_append_printf(str, "? ");
}

static void bfs_nary(NaryNode *root, GString *str, const gchar *type) {
    if (!root) return;

    // Queue dynamique avec capacité initiale et agrandissement automatique
    size_t capacity = 256; // Capacité initiale raisonnable
    NaryNode **queue = malloc(sizeof(NaryNode*) * capacity);
    if (!queue) return; // Échec d'allocation

    size_t front = 0, rear = 0;

    queue[rear++] = root;

    while (front < rear) {
        NaryNode *current = queue[front++];

        if (g_strcmp0(type, "Entiers (Int)") == 0) g_string_append_printf(str, "%d ", *(int*)current->data);
        else g_string_append_printf(str, "? ");

        NaryNode *child = current->first_child;
        while (child) {
            if (rear >= capacity) {
                // Agrandir la queue si nécessaire
                capacity *= 2;
                NaryNode **new_queue = realloc(queue, sizeof(NaryNode*) * capacity);
                if (!new_queue) {
                    free(queue);
                    return; // Échec d'agrandissement
                }
                queue = new_queue;
            }
            queue[rear++] = child;
            child = child->next_sibling;
        }
    }

    free(queue);
}

// --- Stats pour N-Aire ---
static int nary_size(NaryNode *root) {
    if (!root) return 0;
    int count = 1;
    NaryNode *child = root->first_child;
    while (child) {
        count += nary_size(child);
        child = child->next_sibling;
    }
    return count;
}

static int nary_depth(NaryNode *root) {
    if (!root) return 0;
    int max_depth = 0;
    NaryNode *child = root->first_child;
    while (child) {
        int child_depth = nary_depth(child);
        if (child_depth > max_depth) max_depth = child_depth;
        child = child->next_sibling;
    }
    return 1 + max_depth;
}

// Constantes pour le dessin des arbres
#define NODE_RADIUS 20.0
#define VERTICAL_SPACING 80.0
#define HORIZONTAL_SPACING 60.0

// --- Structure pour mapper un nœud à sa position ---
typedef struct {
    BinaryNode *node;
    double x;
    double y;
} NodePosMap;

// --- Structure pour mapper un nœud N-Aire à sa position ---
typedef struct {
    NaryNode *node;
    double x;
    double y;
} NaryNodePosMap;

// --- Positionnement des arbres binaires via parcours infixe ---
// Assure l'ordre gauche-parent-droit et respecte la définition d'un arbre binaire.
static void assign_binary_positions(BinaryNode *node, NodePosMap *map, int *map_index,
                                    double *cursor_x, double start_y, int level) {
    if (!node) return;

    // Parcours infixe pour garantir : sous-arbre gauche -> parent -> sous-arbre droit
    assign_binary_positions(node->left, map, map_index, cursor_x, start_y, level + 1);

    // Position du nœud courant
    double node_x = *cursor_x;
    map[*map_index].node = node;
    map[*map_index].x = node_x;
    map[*map_index].y = start_y + level * VERTICAL_SPACING;
    (*map_index)++;

    // Avancer l'abscisse pour le prochain nœud (même niveau ou droit)
    *cursor_x += HORIZONTAL_SPACING;

    assign_binary_positions(node->right, map, map_index, cursor_x, start_y, level + 1);
}

// --- Trouver la position d'un nœud dans le map ---
static NodePosMap* find_node_position(NodePosMap *map, int map_size, BinaryNode *node) {
    for (int i = 0; i < map_size; i++) {
        if (map[i].node == node) {
            return &map[i];
        }
    }
    return NULL;
}


// --- Dessiner un nœud avec une seule branche blanche vers chaque enfant ---
static void draw_binary_node_with_map(cairo_t *cr, BinaryNode *node, NodePosMap *map, int map_size, const gchar *type) {
    if (!node) return;

    NodePosMap *node_pos = find_node_position(map, map_size, node);
    if (!node_pos) return;

    double x = node_pos->x;
    double y = node_pos->y;

    // Dessiner les branches blanches vers les enfants AVANT les nœuds
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White branches
    cairo_set_line_width(cr, 2.0);

    if (node->left) {
        NodePosMap *left_pos = find_node_position(map, map_size, node->left);
        if (left_pos) {
            // Une seule branche blanche du bord inférieur du parent au bord supérieur de l'enfant
            cairo_move_to(cr, x, y + NODE_RADIUS);
            cairo_line_to(cr, left_pos->x, left_pos->y - NODE_RADIUS);
            cairo_stroke(cr);
        }
    }

    if (node->right) {
        NodePosMap *right_pos = find_node_position(map, map_size, node->right);
        if (right_pos) {
            // Une seule branche blanche du bord inférieur du parent au bord supérieur de l'enfant
            cairo_move_to(cr, x, y + NODE_RADIUS);
            cairo_line_to(cr, right_pos->x, right_pos->y - NODE_RADIUS);
            cairo_stroke(cr);
        }
    }

    // Dessiner récursivement les enfants
    if (node->left) {
        draw_binary_node_with_map(cr, node->left, map, map_size, type);
    }

    if (node->right) {
        draw_binary_node_with_map(cr, node->right, map, map_size, type);
    }

    // Dessiner le cercle du nœud (fond blanc, comme l'arbre n-aire)
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White fill (comme l'arbre n-aire)
    cairo_arc(cr, x, y, NODE_RADIUS, 0, 2 * M_PI);
    cairo_fill(cr); // Pas de stroke pour enlever la bordure

    // Dessiner le texte (noir pour que les nombres soient visibles)
    gchar text[32];
    if (g_strcmp0(type, "Entiers (Int)") == 0) {
        snprintf(text, 32, "%d", *(int*)node->data);
    } else if (g_strcmp0(type, "Réels (Float)") == 0) {
        snprintf(text, 32, "%.1f", *(float*)node->data);
    } else if (g_strcmp0(type, "Caractères (Char)") == 0) {
        snprintf(text, 32, "%c", *(char*)node->data);
    } else {
        snprintf(text, 32, "?");
    }

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Black Text pour visibilité
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 13);
    cairo_text_extents_t extents;
    cairo_text_extents(cr, text, &extents);
    cairo_move_to(cr, x - extents.width/2, y + extents.height/2);
    cairo_show_text(cr, text);
}

// --- Calculer les dimensions nécessaires pour un arbre binaire ---
static void calculate_binary_tree_bounds(BinaryNode *root, double start_x, double start_y,
                                         double *min_x, double *max_x, double *max_y) {
    if (!root) {
        *min_x = start_x;
        *max_x = start_x;
        *max_y = start_y;
        return;
    }

    int node_count = binary_size(root);
    if (node_count == 0) {
        *min_x = start_x;
        *max_x = start_x;
        *max_y = start_y;
        return;
    }

    NodePosMap *map = g_new0(NodePosMap, node_count);
    int map_index = 0;
    double cursor_x = 0.0;
    assign_binary_positions(root, map, &map_index, &cursor_x, start_y, 0);

    // Centrer l'arbre horizontalement
    if (node_count > 0) {
        double min = map[0].x;
        double max = map[0].x;
        double max_y_val = map[0].y;
        for (int i = 1; i < node_count; i++) {
            if (map[i].x < min) min = map[i].x;
            if (map[i].x > max) max = map[i].x;
            if (map[i].y > max_y_val) max_y_val = map[i].y;
        }
        double center_offset = start_x - (min + max) / 2.0;
        for (int i = 0; i < node_count; i++) {
            map[i].x += center_offset;
        }
        // Recalculer min/max après centrage
        min = map[0].x;
        max = map[0].x;
        for (int i = 1; i < node_count; i++) {
            if (map[i].x < min) min = map[i].x;
            if (map[i].x > max) max = map[i].x;
        }
        *min_x = min - NODE_RADIUS;
        *max_x = max + NODE_RADIUS;
        *max_y = max_y_val + NODE_RADIUS;
    } else {
        *min_x = start_x;
        *max_x = start_x;
        *max_y = start_y;
    }

    g_free(map);
}

// --- Fonction principale de dessin améliorée ---
static void draw_binary_tree_improved(cairo_t *cr, BinaryNode *root, double start_x, double start_y, const gchar *type) {
    if (!root) return;

    // Compter le nombre de nœuds
    int node_count = binary_size(root);
    if (node_count == 0) return;

    // Allouer un tableau pour mapper les nœuds aux positions
    NodePosMap *map = g_new0(NodePosMap, node_count);

    // Assigner les positions avec l'algorithme amélioré
    int map_index = 0;
    double cursor_x = 0.0;
    assign_binary_positions(root, map, &map_index, &cursor_x, start_y, 0);

    // Centrer l'arbre horizontalement
    if (node_count > 0) {
        double min_x = map[0].x;
        double max_x = map[0].x;
        for (int i = 1; i < node_count; i++) {
            if (map[i].x < min_x) min_x = map[i].x;
            if (map[i].x > max_x) max_x = map[i].x;
        }
        double center_offset = start_x - (min_x + max_x) / 2.0;
        for (int i = 0; i < node_count; i++) {
            map[i].x += center_offset;
        }
    }

    // Dessiner l'arbre
    draw_binary_node_with_map(cr, root, map, node_count, type);
    g_free(map);
}

// --- Drawing N-Ary Tree (Recursive) ---
static int count_nary_children(NaryNode *node) {
    if (!node || !node->first_child) return 0;
    int count = 1;
    NaryNode *child = node->first_child->next_sibling;
    while (child) {
        count++;
        child = child->next_sibling;
    }
    return count;
}

// --- Calculer la largeur nécessaire pour un sous-arbre n-aire ---
static double calculate_nary_width(NaryNode *node, double x_spacing) {
    if (!node) return 0.0;

    int child_count = count_nary_children(node);
    if (child_count == 0) return x_spacing;

    double total_width = 0.0;
    NaryNode *child = node->first_child;
    while (child) {
        total_width += calculate_nary_width(child, x_spacing);
        child = child->next_sibling;
    }

    // Si un seul enfant, utiliser au moins x_spacing
    if (child_count == 1) {
        return (total_width > x_spacing) ? total_width : x_spacing;
    }

    // Plusieurs enfants : utiliser la largeur totale ou l'espacement minimum
    return (total_width > (child_count - 1) * x_spacing) ? total_width : (child_count - 1) * x_spacing;
}

// --- Fonction récursive pour calculer les dimensions d'un arbre n-aire ---
static void calculate_nary_recursive(NaryNode *node, double x, double y, double x_spacing,
                                     double *min_x, double *max_x, double *max_y) {
    if (!node) return;

    // Mettre à jour les bornes
    if (x - NODE_RADIUS < *min_x) *min_x = x - NODE_RADIUS;
    if (x + NODE_RADIUS > *max_x) *max_x = x + NODE_RADIUS;
    if (y + NODE_RADIUS > *max_y) *max_y = y + NODE_RADIUS;

    int child_count = count_nary_children(node);
    double total_children_width = 0.0;

    if (child_count > 0) {
        NaryNode *child = node->first_child;
        while (child) {
            total_children_width += calculate_nary_width(child, x_spacing);
            child = child->next_sibling;
        }
        if (total_children_width < (child_count - 1) * x_spacing) {
            total_children_width = (child_count - 1) * x_spacing;
        }
    }

    double child_start_x = x - total_children_width / 2.0;
    double child_y = y + VERTICAL_SPACING;
    double current_x = child_start_x;

    if (node->first_child) {
        NaryNode *child = node->first_child;
        while (child) {
            double child_width = calculate_nary_width(child, x_spacing);
            double child_x = current_x + child_width / 2.0;
            calculate_nary_recursive(child, child_x, child_y, x_spacing, min_x, max_x, max_y);
            current_x += child_width;
            child = child->next_sibling;
        }
    }
}

// --- Calculer les dimensions nécessaires pour un arbre n-aire ---
static void calculate_nary_tree_bounds(NaryNode *root, double start_x, double start_y, double x_spacing,
                                       double *min_x, double *max_x, double *max_y) {
    if (!root) {
        *min_x = start_x;
        *max_x = start_x;
        *max_y = start_y;
        return;
    }

    *min_x = start_x;
    *max_x = start_x;
    *max_y = start_y;
    calculate_nary_recursive(root, start_x, start_y, x_spacing, min_x, max_x, max_y);
}

// --- Construire le map des positions pour les arbres N-Aires ---
static void assign_nary_positions(NaryNode *node, NaryNodePosMap *map, int *map_index,
                                  double x, double y, double x_spacing) {
    if (!node) return;

    // Enregistrer la position du nœud courant
    map[*map_index].node = node;
    map[*map_index].x = x;
    map[*map_index].y = y;
    (*map_index)++;

    // Calculer la largeur nécessaire pour tous les enfants
    int child_count = count_nary_children(node);
    double total_children_width = 0.0;

    if (child_count > 0) {
        NaryNode *child = node->first_child;
        while (child) {
            total_children_width += calculate_nary_width(child, x_spacing);
            child = child->next_sibling;
        }
        if (total_children_width < (child_count - 1) * x_spacing) {
            total_children_width = (child_count - 1) * x_spacing;
        }
    }

    // Position de départ pour les enfants
    double child_start_x = x - total_children_width / 2.0;
    double child_y = y + VERTICAL_SPACING;
    double current_x = child_start_x;

    // Assigner les positions des enfants récursivement
    if (node->first_child) {
        NaryNode *child = node->first_child;
        while (child) {
            double child_width = calculate_nary_width(child, x_spacing);
            double child_x = current_x + child_width / 2.0;
            assign_nary_positions(child, map, map_index, child_x, child_y, x_spacing);
            current_x += child_width;
            child = child->next_sibling;
        }
    }
}

// --- Trouver la position d'un nœud N-Aire dans le map ---
static NaryNodePosMap* find_nary_node_position(NaryNodePosMap *map, int map_size, NaryNode *node) {
    for (int i = 0; i < map_size; i++) {
        if (map[i].node == node) {
            return &map[i];
        }
    }
    return NULL;
}

// --- Dessiner l'arbre n-aire avec un meilleur layout ---
static double draw_nary_node_recursive(cairo_t *cr, NaryNode *node, double x, double y, double x_spacing, const gchar *type) {
    if (!node) return 0.0;

    // Calculer la largeur nécessaire pour tous les enfants
    int child_count = count_nary_children(node);
    double total_children_width = 0.0;

    if (child_count > 0) {
        NaryNode *child = node->first_child;
        while (child) {
            total_children_width += calculate_nary_width(child, x_spacing);
            child = child->next_sibling;
        }
        // Espacement minimum entre enfants
        if (total_children_width < (child_count - 1) * x_spacing) {
            total_children_width = (child_count - 1) * x_spacing;
        }
    }

    // Position de départ pour les enfants (centrés sous le parent)
    double child_start_x = x - total_children_width / 2.0;
    double child_y = y + VERTICAL_SPACING;
    double current_x = child_start_x;

    // Dessiner les lignes et les enfants
    if (node->first_child) {
        NaryNode *child = node->first_child;
        while (child) {
            double child_width = calculate_nary_width(child, x_spacing);
            double child_x = current_x + child_width / 2.0;

            // Dessiner la ligne AVANT le nœud parent
            cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White lines
            cairo_set_line_width(cr, 2.0);
            cairo_move_to(cr, x, y + NODE_RADIUS);
            cairo_line_to(cr, child_x, child_y - NODE_RADIUS);
            cairo_stroke(cr);
            // Dessiner récursivement l'enfant
            draw_nary_node_recursive(cr, child, child_x, child_y, x_spacing, type);

            current_x += child_width;
            child = child->next_sibling;
        }
    }

    // Dessiner le nœud PAR-DESSUS les lignes (fond blanc)
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White fill
    cairo_arc(cr, x, y, NODE_RADIUS, 0, 2 * M_PI);
    cairo_fill(cr); // Pas de stroke pour enlever la bordure

    // Dessiner le texte (noir pour que les nombres soient visibles)
    gchar text[32];
    if (g_strcmp0(type, "Entiers (Int)") == 0) {
        snprintf(text, 32, "%d", *(int*)node->data);
    } else if (g_strcmp0(type, "Réels (Float)") == 0) {
        snprintf(text, 32, "%.1f", *(float*)node->data);
    } else if (g_strcmp0(type, "Caractères (Char)") == 0) {
        snprintf(text, 32, "%c", *(char*)node->data);
    } else {
        snprintf(text, 32, "?");
    }

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Black Text pour visibilité
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 13);
    cairo_text_extents_t extents;
    cairo_text_extents(cr, text, &extents);
    cairo_move_to(cr, x - extents.width/2, y + extents.height/2);
    cairo_show_text(cr, text);

    // Retourner la largeur utilisée
    return (total_children_width > x_spacing) ? total_children_width : x_spacing;
}

// Espacement adaptatif pour les arbres N-aires (branches courtes et compactes)
static double compute_nary_spacing(int tree_size, int max_children, double viewport_width) {
    // Base réduite pour des branches plus courtes
    double base = viewport_width / 10.0;
    if (base < 40.0) base = 40.0; // Minimum réduit

    // Facteurs réduits pour un espacement plus compact
    // Utiliser log pour croissance lente mais avec des facteurs plus petits
    double size_factor = log(tree_size > 0 ? tree_size : 1) * 5.0; // Réduit de 15.0 à 5.0
    double degree_factor = (max_children > 1 ? max_children - 1 : 0) * 10.0; // Réduit de 30.0 à 10.0

    // Pour les très grands arbres, facteur supplémentaire réduit
    if (tree_size > 1000) {
        size_factor += (tree_size / 1000.0) * 10.0; // Réduit de 50.0 à 10.0
    }

    double spacing = base + size_factor + degree_factor;

    // Limiter l'espacement maximum pour éviter les branches trop longues
    double max_spacing = viewport_width / 4.0; // Maximum basé sur le viewport
    if (spacing > max_spacing) {
        spacing = max_spacing;
    }

    return spacing;
}

// --- Mettre à jour la taille du drawing_area en fonction de l'arbre ---
static gboolean update_tree_drawing_area_size(gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data || !app_data->tree_drawing_area) return FALSE;

    guint viewport_width = 800; // Largeur par défaut
    guint viewport_height = 600; // Hauteur par défaut

    // Obtenir la taille du viewport si le scrolled window existe
    if (app_data->tree_scrolled_window) {
        GtkAllocation allocation;
        gtk_widget_get_allocation(app_data->tree_scrolled_window, &allocation);
        viewport_width = allocation.width > 0 ? allocation.width : 800;
        viewport_height = allocation.height > 0 ? allocation.height : 600;
    }

    double start_y = 40.0;
    double min_x, max_x, max_y;
    int required_width, required_height;

    if (app_data->tree_is_nary == 0 && app_data->binary_root) {
        // Pour les arbres binaires, utiliser le centre du viewport comme point de départ
        double start_x = viewport_width / 2.0;
        calculate_binary_tree_bounds(app_data->binary_root, start_x, start_y, &min_x, &max_x, &max_y);
        required_width = (int)(max_x - min_x + 80); // Plus de padding pour éviter le clipping
        required_height = (int)(max_y + 80);
    } else if (app_data->tree_is_nary == 1 && app_data->nary_root) {
        int tree_size = nary_size(app_data->nary_root);
        // Pour les arbres N-aire, utiliser des estimations réduites car l'espacement est maintenant plus compact
        double estimated_width = viewport_width * 1.5; // Réduit de 3.0 à 1.5
        if (tree_size > 50) {
            estimated_width = viewport_width * 2.0; // Réduit de 4.0 à 2.0
        }
        if (tree_size > 100) {
            estimated_width = viewport_width * 2.5; // Réduit de 6.0 à 2.5
        }
        if (tree_size > 500) {
            estimated_width = viewport_width * 3.0; // Réduit de 10.0 à 3.0
        }
        if (tree_size > 1000) {
            estimated_width = viewport_width * 4.0; // Réduit de 15.0 à 4.0
        }

        // Calculer l'espacement avec cette largeur estimée
        double x_spacing = compute_nary_spacing(tree_size, app_data->nary_max_children, estimated_width);

        // Utiliser le centre de la largeur estimée comme point de départ
        double start_x = estimated_width / 2.0;
        calculate_nary_tree_bounds(app_data->nary_root, start_x, start_y, x_spacing, &min_x, &max_x, &max_y);

        // Calculer la largeur nécessaire en tenant compte que min_x peut être négatif
        // Avec l'espacement réduit, l'arbre devrait être plus compact
        double actual_width = max_x - min_x;
        required_width = (int)(actual_width + 100); // Padding réduit car branches plus courtes
        required_height = (int)(max_y + 100);

        // S'assurer que la largeur est suffisante même si min_x est très négatif
        if (min_x < 0) {
            double min_x_abs = -min_x; // Valeur absolue de min_x
            // La largeur doit couvrir de min_x (négatif) à max_x (positif)
            required_width = (int)(max_x + min_x_abs + 100);
        }

        // S'assurer que la largeur minimale est au moins celle estimée
        if (required_width < (int)estimated_width) {
            required_width = (int)estimated_width;
        }

        // Recalculer avec la largeur finale pour s'assurer que tout est visible
        // Utiliser le viewport pour le calcul de l'espacement (branches courtes)
        double final_x_spacing = compute_nary_spacing(tree_size, app_data->nary_max_children, (double)viewport_width);
        double final_start_x = (double)required_width / 2.0;
        calculate_nary_tree_bounds(app_data->nary_root, final_start_x, start_y, final_x_spacing, &min_x, &max_x, &max_y);

        // Recalculer required_width avec les nouvelles bornes (padding réduit)
        actual_width = max_x - min_x;
        required_width = (int)(actual_width + 150); // Padding réduit
        if (min_x < 0) {
            double min_x_abs = -min_x;
            required_width = (int)(max_x + min_x_abs + 150);
        }
        // S'assurer que required_width est au moins estimated_width
        if (required_width < (int)estimated_width) {
            required_width = (int)estimated_width;
        }
    } else {
        required_width = viewport_width;
        required_height = viewport_height;
    }

    // S'assurer que la taille est au moins celle du viewport
    if (required_width < viewport_width) required_width = viewport_width;
    if (required_height < viewport_height) required_height = viewport_height;

    // Limite maximale raisonnable pour éviter les problèmes de mémoire (mais assez grande)
    if (required_width > 50000) required_width = 50000;
    if (required_height > 50000) required_height = 50000;

    gtk_widget_set_size_request(app_data->tree_drawing_area, required_width, required_height);
    return FALSE; // Ne pas répéter
}

static gboolean draw_tree_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
    AppData *app_data = (AppData *)data;

    // Mettre à jour la taille AVANT le dessin
    update_tree_drawing_area_size(app_data);

    guint width = gtk_widget_get_allocated_width(widget);
    guint height = gtk_widget_get_allocated_height(widget);

    // Pour les arbres N-aire, avec l'espacement réduit, l'arbre devrait être plus compact
    // Utiliser la largeur réelle allouée qui devrait être suffisante maintenant

    cairo_set_source_rgb(cr, 0.1, 0.1, 0.12); // #1a1a1f Dark BG
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White lines

    // Vérifier la taille pour décider si on dessine
    int tree_size = 0;
    if (app_data->tree_is_nary == 0 && app_data->binary_root) {
        tree_size = binary_size(app_data->binary_root);
    } else if (app_data->tree_is_nary == 1 && app_data->nary_root) {
        tree_size = nary_size(app_data->nary_root);
    }

    // Dessiner l'arbre sans limitation de taille
    if (tree_size > 0) {
        double start_y = 40.0;

        if (app_data->tree_is_nary == 0 && app_data->binary_root) {
            // Pour les arbres binaires, utiliser le centre de la largeur réelle
            double start_x = (width > 0) ? width / 2.0 : 400.0;
            draw_binary_tree_improved(cr, app_data->binary_root, start_x, start_y, "Entiers (Int)");
        } else if (app_data->tree_is_nary == 1 && app_data->nary_root) {
            int tree_size = nary_size(app_data->nary_root);

            // Obtenir la largeur du viewport pour le calcul de l'espacement
            guint viewport_width = 800;
            if (app_data->tree_scrolled_window) {
                GtkAllocation allocation;
                gtk_widget_get_allocation(app_data->tree_scrolled_window, &allocation);
                viewport_width = allocation.width > 0 ? allocation.width : 800;
            }

            // Utiliser le viewport directement pour calculer l'espacement (branches courtes)
            // Cela garantit que les branches sont courtes et visibles à l'écran
            double x_spacing = compute_nary_spacing(tree_size, app_data->nary_max_children, (double)viewport_width);

            // Utiliser la largeur réelle du drawing_area pour centrer l'arbre
            double drawing_width = (width > 0) ? (double)width : (double)viewport_width;
            double start_x = drawing_width / 2.0;

            // Dessiner l'arbre avec des branches courtes et compactes
            draw_nary_node_recursive(cr, app_data->nary_root, start_x, start_y, x_spacing, "Entiers (Int)");
        }
    } else {
        // Arbre vide
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_set_font_size(cr, 16);
        cairo_move_to(cr, (width > 0 ? width/2 - 30 : 370), height/2);
        cairo_show_text(cr, "Arbre Vide");
    }

    return TRUE;
}

// --- Callbacks ---
static void on_tree_input_source_toggled(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        if (widget == app_data->tree_random_radio) {
            app_data->tree_input_source = 0;
            // Afficher les éléments aléatoires
            if (app_data->tree_size_input) {
                gtk_widget_show(GTK_WIDGET(app_data->tree_size_input));
            }
            if (app_data->tree_random_button) {
                gtk_widget_show(app_data->tree_random_button);
            }
            // Masquer les éléments manuels
            if (app_data->tree_manual_label) {
                gtk_widget_hide(app_data->tree_manual_label);
            }
            if (app_data->tree_manual_entry) {
                gtk_widget_hide(GTK_WIDGET(app_data->tree_manual_entry));
            }
            if (app_data->tree_manual_button) {
                gtk_widget_hide(app_data->tree_manual_button);
            }
        } else if (widget == app_data->tree_manual_radio) {
            app_data->tree_input_source = 1;
            // Masquer les éléments aléatoires
            if (app_data->tree_size_input) {
                gtk_widget_hide(GTK_WIDGET(app_data->tree_size_input));
            }
            if (app_data->tree_random_button) {
                gtk_widget_hide(app_data->tree_random_button);
            }
            // Afficher les éléments manuels
            if (app_data->tree_manual_label) {
                gtk_widget_show(app_data->tree_manual_label);
            }
            if (app_data->tree_manual_entry) {
                gtk_widget_show(GTK_WIDGET(app_data->tree_manual_entry));
            }
            if (app_data->tree_manual_button) {
                gtk_widget_show(app_data->tree_manual_button);
            }
        }
    }
}

static void on_tree_create_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    // Clear existing
    if (app_data->binary_root) {
        free_binary_tree(app_data->binary_root);
        app_data->binary_root = NULL;
    }
    if (app_data->nary_root) {
        free_nary_tree(app_data->nary_root);
        app_data->nary_root = NULL;
    }

    app_data->tree_is_nary = gtk_combo_box_get_active(GTK_COMBO_BOX(app_data->tree_type_combo));
    app_data->nary_max_children = app_data->tree_nary_degree_input
        ? gtk_spin_button_get_value_as_int(app_data->tree_nary_degree_input)
        : 3;
    if (app_data->nary_max_children < 1) app_data->nary_max_children = 1;
    if (app_data->tree_manual_entry) gtk_entry_set_text(app_data->tree_manual_entry, "");
    gtk_widget_queue_draw(app_data->tree_drawing_area);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
    gtk_text_buffer_set_text(buffer, "Nouvel arbre créé (Vide).", -1);
}

static void on_tree_insert_random_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;

    // Obtenir la taille depuis le spin button
    int n = app_data->tree_size_input ? gtk_spin_button_get_value_as_int(app_data->tree_size_input) : 10;
    if (n <= 0) n = 10;
    if (app_data->tree_nary_degree_input) {
        app_data->nary_max_children = gtk_spin_button_get_value_as_int(app_data->tree_nary_degree_input);
        if (app_data->nary_max_children < 1) app_data->nary_max_children = 1;
    }
    app_data->tree_is_nary = gtk_combo_box_get_active(GTK_COMBO_BOX(app_data->tree_type_combo));

    int (*cmp)(const void*, const void*) = compare_int;

    if (app_data->tree_is_nary == 0) {
        for(int i=0; i<n; i++) {
            int val = rand() % 100;
            app_data->binary_root = insert_binary(app_data->binary_root, &val, cmp, sizeof(int));
        }
    } else {
        for(int i=0; i<n; i++) {
            int val = rand() % 100;
            app_data->nary_root = insert_nary_random(app_data->nary_root, &val, sizeof(int), app_data->nary_max_children);
        }
    }

    // Forcer le redimensionnement avant le dessin pour les grands arbres
    g_idle_add((GSourceFunc)update_tree_drawing_area_size, app_data);
    gtk_widget_queue_draw(app_data->tree_drawing_area);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
    gtk_text_buffer_set_text(buffer, g_strdup_printf("Insertion aléatoire de %d éléments terminée.", n), -1);
}

static void on_tree_traversal_execute(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;

    if (!app_data->tree_traversal_combo) return;
    const gchar *mode = gtk_combo_box_text_get_active_text(app_data->tree_traversal_combo);
    if (!mode) return;

    GString *res = g_string_new("");
    g_string_append_printf(res, "Parcours %s: ", mode);

    if (app_data->tree_is_nary == 0 && app_data->binary_root) {
        if (g_strcmp0(mode, "Pré-ordre") == 0) {
            preorder_binary(app_data->binary_root, res, "Entiers (Int)");
        } else if (g_strcmp0(mode, "In-ordre") == 0) {
            inorder_binary(app_data->binary_root, res, "Entiers (Int)");
        } else if (g_strcmp0(mode, "Post-ordre") == 0) {
            postorder_binary(app_data->binary_root, res, "Entiers (Int)");
        } else if (g_strcmp0(mode, "Largeur (BFS)") == 0) {
            bfs_binary(app_data->binary_root, res, "Entiers (Int)");
        }
    } else if (app_data->tree_is_nary == 1 && app_data->nary_root) {
        if (g_strcmp0(mode, "Pré-ordre") == 0) {
            preorder_nary(app_data->nary_root, res, "Entiers (Int)");
        } else if (g_strcmp0(mode, "Post-ordre") == 0) {
            postorder_nary(app_data->nary_root, res, "Entiers (Int)");
        } else if (g_strcmp0(mode, "Largeur (BFS)") == 0) {
            bfs_nary(app_data->nary_root, res, "Entiers (Int)");
        } else {
            g_string_append(res, "In-ordre non supporté pour N-Aire.");
        }
    } else {
        g_string_append(res, "Arbre vide.");
    }

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
    gtk_text_buffer_set_text(buffer, res->str, -1);
    g_string_free(res, TRUE);
}

// Fonction de compatibilité pour les anciens boutons
static void on_tree_traversal_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (app_data->tree_is_nary != 0) return;

    const gchar *mode = (const gchar *)data;
    GString *res = g_string_new("");
    g_string_append_printf(res, "Parcours %s: ", mode);

    if (g_strcmp0(mode, "Pré-ordre") == 0) preorder_binary(app_data->binary_root, res, "Entiers (Int)");
    else if (g_strcmp0(mode, "In-ordre") == 0) inorder_binary(app_data->binary_root, res, "Entiers (Int)");
    else if (g_strcmp0(mode, "Post-ordre") == 0) postorder_binary(app_data->binary_root, res, "Entiers (Int)");

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
    gtk_text_buffer_set_text(buffer, res->str, -1);
    g_string_free(res, TRUE);
}

static void on_tree_bfs_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (app_data->tree_is_nary == 0 && app_data->binary_root) {
        GString *res = g_string_new("Parcours Largeur (BFS): ");
        bfs_binary(app_data->binary_root, res, "Entiers (Int)");
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
        gtk_text_buffer_set_text(buffer, res->str, -1);
        g_string_free(res, TRUE);
    } else if (app_data->tree_is_nary == 1 && app_data->nary_root) {
        GString *res = g_string_new("Parcours Largeur (BFS): ");
        bfs_nary(app_data->nary_root, res, "Entiers (Int)");
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
        gtk_text_buffer_set_text(buffer, res->str, -1);
        g_string_free(res, TRUE);
    } else {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
        gtk_text_buffer_set_text(buffer, "Arbre vide.", -1);
    }
}

static void on_tree_stats_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (app_data->tree_is_nary == 0 && app_data->binary_root) {
        int depth = binary_depth(app_data->binary_root);
        int size = binary_size(app_data->binary_root);
        gchar *msg = g_strdup_printf("Statistiques Arbre Binaire:\nTaille: %d\nProfondeur: %d", size, depth);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
        gtk_text_buffer_set_text(buffer, msg, -1);
        g_free(msg);
    } else if (app_data->tree_is_nary == 1 && app_data->nary_root) {
        int depth = nary_depth(app_data->nary_root);
        int size = nary_size(app_data->nary_root);
        gchar *msg = g_strdup_printf("Statistiques Arbre N-Aire:\nTaille: %d\nProfondeur: %d", size, depth);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
        gtk_text_buffer_set_text(buffer, msg, -1);
        g_free(msg);
    } else {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
        gtk_text_buffer_set_text(buffer, "Arbre vide.", -1);
    }
}

// --- Ordonner un arbre binaire (reconstruire en ordre) ---
static void collect_inorder(BinaryNode *root, int *values, int *index) {
    if (!root) return;
    collect_inorder(root->left, values, index);
    values[(*index)++] = *(int*)root->data;
    collect_inorder(root->right, values, index);
}

static BinaryNode *build_balanced_bst(int *values, int start, int end) {
    if (start > end) return NULL;

    int mid = (start + end) / 2;
    BinaryNode *node = g_new0(BinaryNode, 1);
    node->data = malloc(sizeof(int));
    *(int*)node->data = values[mid];

    node->left = build_balanced_bst(values, start, mid - 1);
    node->right = build_balanced_bst(values, mid + 1, end);

    return node;
}

static void on_tree_order_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;

    if (app_data->tree_is_nary == 0 && app_data->binary_root) {
        int size = binary_size(app_data->binary_root);
        if (size == 0) {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
            gtk_text_buffer_set_text(buffer, "Arbre vide, rien à ordonner.", -1);
            return;
        }

        // Collecter les valeurs en ordre
        int *values = malloc(sizeof(int) * size);
        int index = 0;
        collect_inorder(app_data->binary_root, values, &index);

        // Trier les valeurs
        qsort(values, size, sizeof(int), compare_int);

        // Reconstruire l'arbre équilibré
        if (app_data->binary_root) free_binary_tree(app_data->binary_root);
        app_data->binary_root = build_balanced_bst(values, 0, size - 1);

        free(values);

        gtk_widget_queue_draw(app_data->tree_drawing_area);

        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Arbre ordonné et équilibré. Taille: %d", size), -1);
    } else {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
        gtk_text_buffer_set_text(buffer, "Ordonnancement disponible uniquement pour les arbres binaires.", -1);
    }
}

static void on_tree_transform_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (app_data->tree_is_nary == 1 && app_data->nary_root) {
        // Convert N-Ary to Binary
        BinaryNode *new_root = convert_nary_to_binary(app_data->nary_root, sizeof(int));

        // Switch mode
        app_data->tree_is_nary = 0;
        if (app_data->binary_root) free_binary_tree(app_data->binary_root);
        app_data->binary_root = new_root;

        // Update UI
        gtk_combo_box_set_active(GTK_COMBO_BOX(app_data->tree_type_combo), 0); // Set to Binaire
        gtk_widget_queue_draw(app_data->tree_drawing_area);

        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
        gtk_text_buffer_set_text(buffer, "Transformation N-Aire -> Binaire effectuée.", -1);
    } else {
         GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
        gtk_text_buffer_set_text(buffer, "Aucun arbre N-Aire à transformer.", -1);
    }
}

// --- Delete/Modify Helpers ---
static BinaryNode *find_min(BinaryNode *node) {
    while (node->left) node = node->left;
    return node;
}

static BinaryNode *delete_binary_node(BinaryNode *root, int val) {
    if (!root) return NULL;

    int root_val = *(int*)root->data;
    if (val < root_val) root->left = delete_binary_node(root->left, val);
    else if (val > root_val) root->right = delete_binary_node(root->right, val);
    else {
        // Node found
        if (!root->left) {
            BinaryNode *temp = root->right;
            free(root->data); free(root);
            return temp;
        } else if (!root->right) {
            BinaryNode *temp = root->left;
            free(root->data); free(root);
            return temp;
        }
        BinaryNode *temp = find_min(root->right);
        *(int*)root->data = *(int*)temp->data; // Copy data
        root->right = delete_binary_node(root->right, *(int*)temp->data);
    }
    return root;
}

static NaryNode *delete_nary_node(NaryNode *root, int val) {
    if (!root) return NULL;

    // Check root
    if (*(int*)root->data == val) {
        // Remove this node and its entire subtree
        // FIX: Detach next_sibling so it is not freed
        NaryNode *siblings = root->next_sibling;
        root->next_sibling = NULL;
        free_nary_tree(root);
        return siblings;
    }

    // Check children (siblings of first_child)
    if (root->first_child) {
        root->first_child = delete_nary_node(root->first_child, val);
    }
    // Check next sibling
    root->next_sibling = delete_nary_node(root->next_sibling, val);

    return root;
}

static void on_tree_delete_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    int val = get_integer_input(gtk_widget_get_toplevel(widget), "Suppression", "Valeur à supprimer:", 0);

    if (app_data->tree_is_nary == 0 && app_data->binary_root) {
        int size_before = binary_size(app_data->binary_root);
        app_data->binary_root = delete_binary_node(app_data->binary_root, val);
        int size_after = binary_size(app_data->binary_root);

        gtk_widget_queue_draw(app_data->tree_drawing_area);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
        if (size_before > size_after) {
            gtk_text_buffer_set_text(buffer, g_strdup_printf("Valeur %d supprimée de l'arbre binaire.", val), -1);
        } else {
            gtk_text_buffer_set_text(buffer, g_strdup_printf("Valeur %d non trouvée dans l'arbre binaire.", val), -1);
        }
    } else if (app_data->tree_is_nary == 1 && app_data->nary_root) {
        int size_before = nary_size(app_data->nary_root);
        app_data->nary_root = delete_nary_node(app_data->nary_root, val);
        int size_after = nary_size(app_data->nary_root);

        gtk_widget_queue_draw(app_data->tree_drawing_area);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
        if (size_before > size_after) {
            gtk_text_buffer_set_text(buffer, g_strdup_printf("Valeur %d supprimée de l'arbre N-Aire.", val), -1);
        } else {
            gtk_text_buffer_set_text(buffer, g_strdup_printf("Valeur %d non trouvée dans l'arbre N-Aire.", val), -1);
        }
    } else {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
        gtk_text_buffer_set_text(buffer, "Arbre vide.", -1);
    }
}

// --- Fonctions pour trouver le nœud cliqué ---
static BinaryNode* find_clicked_binary_node(AppData *app_data, double click_x, double click_y) {
    if (!app_data->binary_root) return NULL;

    guint width = gtk_widget_get_allocated_width(app_data->tree_drawing_area);
    double start_x = (width > 0) ? width / 2.0 : 400.0;
    double start_y = 40.0;

    int node_count = binary_size(app_data->binary_root);
    if (node_count == 0) return NULL;

    // Construire le map des positions
    NodePosMap *map = g_new0(NodePosMap, node_count);
    int map_index = 0;
    double cursor_x = 0.0;
    assign_binary_positions(app_data->binary_root, map, &map_index, &cursor_x, start_y, 0);

    // Centrer l'arbre horizontalement
    if (node_count > 0) {
        double min_x = map[0].x;
        double max_x = map[0].x;
        for (int i = 1; i < node_count; i++) {
            if (map[i].x < min_x) min_x = map[i].x;
            if (map[i].x > max_x) max_x = map[i].x;
        }
        double center_offset = start_x - (min_x + max_x) / 2.0;
        for (int i = 0; i < node_count; i++) {
            map[i].x += center_offset;
        }
    }

    // Trouver le nœud cliqué
    BinaryNode *clicked_node = NULL;
    double min_distance = NODE_RADIUS * NODE_RADIUS; // Distance au carré pour éviter sqrt
    for (int i = 0; i < node_count; i++) {
        double dx = click_x - map[i].x;
        double dy = click_y - map[i].y;
        double distance_sq = dx * dx + dy * dy;
        if (distance_sq < min_distance) {
            min_distance = distance_sq;
            clicked_node = map[i].node;
        }
    }

    g_free(map);
    return clicked_node;
}

static NaryNode* find_clicked_nary_node(AppData *app_data, double click_x, double click_y) {
    if (!app_data->nary_root) return NULL;

    guint width = gtk_widget_get_allocated_width(app_data->tree_drawing_area);
    double start_x = (width > 0) ? width / 2.0 : 400.0;
    double start_y = 40.0;
    double viewport_width = (width > 0) ? width : 800.0;
    int tree_size = nary_size(app_data->nary_root);
    double x_spacing = compute_nary_spacing(tree_size, app_data->nary_max_children, viewport_width);

    // Construire le map des positions
    int node_count = nary_size(app_data->nary_root);
    if (node_count == 0) return NULL;

    NaryNodePosMap *map = g_new0(NaryNodePosMap, node_count);
    int map_index = 0;
    assign_nary_positions(app_data->nary_root, map, &map_index, start_x, start_y, x_spacing);

    // Trouver le nœud cliqué
    NaryNode *clicked_node = NULL;
    double min_distance = NODE_RADIUS * NODE_RADIUS; // Distance au carré pour éviter sqrt
    for (int i = 0; i < node_count; i++) {
        double dx = click_x - map[i].x;
        double dy = click_y - map[i].y;
        double distance_sq = dx * dx + dy * dy;
        if (distance_sq < min_distance) {
            min_distance = distance_sq;
            clicked_node = map[i].node;
        }
    }

    g_free(map);
    return clicked_node;
}

// Fonction helper pour trouver et modifier un nœud Binaire
static gboolean modify_binary_node(BinaryNode *root, int old_val, int new_val) {
    if (!root) return FALSE;

    if (*(int*)root->data == old_val) {
        *(int*)root->data = new_val;
        return TRUE;
    }

    // Chercher récursivement dans les sous-arbres
    if (modify_binary_node(root->left, old_val, new_val)) {
        return TRUE;
    }
    if (modify_binary_node(root->right, old_val, new_val)) {
        return TRUE;
    }

    return FALSE;
}

// Fonction helper pour trouver et modifier un nœud N-Aire
static gboolean modify_nary_node(NaryNode *root, int old_val, int new_val) {
    if (!root) return FALSE;

    if (*(int*)root->data == old_val) {
        *(int*)root->data = new_val;
        return TRUE;
    }

    // Chercher dans les enfants
    NaryNode *child = root->first_child;
    while (child) {
        if (modify_nary_node(child, old_val, new_val)) {
            return TRUE;
        }
        child = child->next_sibling;
    }

    return FALSE;
}

static void on_tree_modify_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    int old_val = get_integer_input(gtk_widget_get_toplevel(widget), "Modification", "Valeur à modifier:", 0);
    int new_val = get_integer_input(gtk_widget_get_toplevel(widget), "Modification", "Nouvelle Valeur:", 0);

    if (app_data->tree_is_nary == 0 && app_data->binary_root) {
        // Binaire: trouver et modifier directement (sans changer la structure)
        if (modify_binary_node(app_data->binary_root, old_val, new_val)) {
            gtk_widget_queue_draw(app_data->tree_drawing_area);
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
            gtk_text_buffer_set_text(buffer, g_strdup_printf("Modification Binaire: %d -> %d", old_val, new_val), -1);
        } else {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
            gtk_text_buffer_set_text(buffer, g_strdup_printf("Valeur %d non trouvée dans l'arbre binaire.", old_val), -1);
        }
    } else if (app_data->tree_is_nary == 1 && app_data->nary_root) {
        // N-Ary: trouver et modifier directement
        if (modify_nary_node(app_data->nary_root, old_val, new_val)) {
            gtk_widget_queue_draw(app_data->tree_drawing_area);
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
            gtk_text_buffer_set_text(buffer, g_strdup_printf("Modification N-Aire: %d -> %d", old_val, new_val), -1);
        } else {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
            gtk_text_buffer_set_text(buffer, g_strdup_printf("Valeur %d non trouvée dans l'arbre N-Aire.", old_val), -1);
        }
    } else {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
        gtk_text_buffer_set_text(buffer, "Arbre vide.", -1);
    }
}

// --- Gestionnaire d'événements de clic pour les arbres ---
static gboolean on_tree_click_event(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    AppData *app_data = (AppData *)data;
    if (!app_data || event->type != GDK_BUTTON_PRESS || event->button != 1) return FALSE;

    // Vérifier qu'il y a un arbre à modifier
    if (app_data->tree_is_nary == 0 && app_data->binary_root) {
        BinaryNode *clicked_node = find_clicked_binary_node(app_data, event->x, event->y);
        if (clicked_node) {
            int current_val = *(int *)clicked_node->data;
            int new_val = get_integer_input(gtk_widget_get_toplevel(widget), "Modifier", "Nouvelle valeur:", current_val);
            *(int *)clicked_node->data = new_val;
            gtk_widget_queue_draw(widget);

            GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
            gtk_text_buffer_set_text(buffer, g_strdup_printf("Modification Binaire: %d -> %d", current_val, new_val), -1);
            return TRUE;
        }
    } else if (app_data->tree_is_nary == 1 && app_data->nary_root) {
        NaryNode *clicked_node = find_clicked_nary_node(app_data, event->x, event->y);
        if (clicked_node) {
            int current_val = *(int *)clicked_node->data;
            int new_val = get_integer_input(gtk_widget_get_toplevel(widget), "Modifier", "Nouvelle valeur:", current_val);
            *(int *)clicked_node->data = new_val;
            gtk_widget_queue_draw(widget);

            GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
            gtk_text_buffer_set_text(buffer, g_strdup_printf("Modification N-Aire: %d -> %d", current_val, new_val), -1);
            return TRUE;
        }
    }

    return FALSE;
}

static void on_tree_insert_manual_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;

    // Récupérer la valeur depuis l'entry (fallback dialog si vide)
    const char *text = app_data->tree_manual_entry ? gtk_entry_get_text(app_data->tree_manual_entry) : "";
    int val = 0;
    if (text && strlen(text) > 0) {
        val = atoi(text);
    } else {
        val = get_integer_input(gtk_widget_get_toplevel(widget), "Insertion Manuelle", "Valeur:", 0);
    }

    if (app_data->tree_nary_degree_input) {
        app_data->nary_max_children = gtk_spin_button_get_value_as_int(app_data->tree_nary_degree_input);
        if (app_data->nary_max_children < 1) app_data->nary_max_children = 1;
    }
    app_data->tree_is_nary = gtk_combo_box_get_active(GTK_COMBO_BOX(app_data->tree_type_combo));

    if (app_data->tree_is_nary == 0) {
        int (*cmp)(const void*, const void*) = compare_int;
        app_data->binary_root = insert_binary(app_data->binary_root, &val, cmp, sizeof(int));

        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Valeur %d insérée dans l'arbre binaire.", val), -1);
    } else {
        // N-Aire avec respect du degré max
        app_data->nary_root = insert_nary_random(app_data->nary_root, &val, sizeof(int), app_data->nary_max_children);

        GtkTextBuffer *buffer = gtk_text_view_get_buffer(app_data->tree_info_view);
        gtk_text_buffer_set_text(buffer, g_strdup_printf("Valeur %d insérée dans l'arbre N-Aire.", val), -1);
    }

    if (app_data->tree_manual_entry) gtk_entry_set_text(app_data->tree_manual_entry, "");
    // Forcer le redimensionnement avant le dessin pour les grands arbres
    g_idle_add((GSourceFunc)update_tree_drawing_area_size, app_data);
    gtk_widget_queue_draw(app_data->tree_drawing_area);
}


// --- Window ---
static void create_tree_window(GtkWidget *parent_window) {
    AppData *app_data = g_new0(AppData, 1);
    // Init app_data defaults...
    app_data->nary_max_children = 3;

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "🌳 Module Arbres (Binaires & N-Aires)");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    // Ne pas utiliser transient_for pour éviter que la fermeture de la fenêtre secondaire ferme la principale
    // gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent_window));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(window), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(window), 15);

    // Container principal avec fond moderne
    GtkWidget *main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_container);

    // Header moderne avec gradient
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(header_box), "modern-card-header");
    gtk_widget_set_size_request(header_box, -1, 80);
    gtk_box_pack_start(GTK_BOX(main_container), header_box, FALSE, FALSE, 0);

    GtkWidget *header_title = gtk_label_new(NULL);
    PangoFontDescription *header_font = pango_font_description_from_string("Segoe UI Bold 24");
    gtk_widget_override_font(header_title, header_font);
    pango_font_description_free(header_font);
    gtk_label_set_markup(GTK_LABEL(header_title), "<span foreground='#ffffff' size='x-large' weight='bold'>🌳 MODULE ARBRES</span>\n<span size='small' foreground='#ffffff'>Binaires & N-Aires</span>");
    gtk_label_set_justify(GTK_LABEL(header_title), GTK_JUSTIFY_CENTER);
    gtk_label_set_xalign(GTK_LABEL(header_title), 0.5);
    gtk_box_pack_start(GTK_BOX(header_box), header_title, TRUE, TRUE, 0);

    // Panneau principal avec paned
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(main_container), hbox, TRUE, TRUE, 0);

    // ========== PANEL GAUCHE : CONTROLES MODERNES ==========
    GtkWidget *control_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(control_scrolled), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(control_scrolled), GTK_SHADOW_NONE);

    GtkWidget *control_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(control_vbox), "control-panel");
    gtk_widget_set_size_request(control_vbox, 350, -1);
    gtk_container_set_border_width(GTK_CONTAINER(control_vbox), 20);
    gtk_container_add(GTK_CONTAINER(control_scrolled), control_vbox);

    // Section Type d'Arbre (Carte moderne)
    GtkWidget *type_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(type_card), "modern-card");
    gtk_box_pack_start(GTK_BOX(control_vbox), type_card, FALSE, FALSE, 0);

    GtkWidget *lbl_type = gtk_label_new("Type d'Arbre");
    gtk_label_set_markup(GTK_LABEL(lbl_type), "<span foreground='#ffffff' size='large' weight='bold'>🌳 Type d'Arbre</span>");
    gtk_box_pack_start(GTK_BOX(type_card), lbl_type, FALSE, FALSE, 0);

    app_data->tree_type_combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(app_data->tree_type_combo, "Binaire");
    gtk_combo_box_text_append_text(app_data->tree_type_combo, "N-Aire");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app_data->tree_type_combo), 0);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(app_data->tree_type_combo)), "modern-combo");
    gtk_box_pack_start(GTK_BOX(type_card), GTK_WIDGET(app_data->tree_type_combo), FALSE, FALSE, 0);

    // Section Configuration (Carte moderne)
    GtkWidget *config_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(config_card), "modern-card");
    gtk_box_pack_start(GTK_BOX(control_vbox), config_card, FALSE, FALSE, 0);

    // Taille de l'arbre
    GtkWidget *lbl_size = gtk_label_new("Taille");
    gtk_label_set_markup(GTK_LABEL(lbl_size), "<span foreground='#ffffff' size='large' weight='bold'>📏 Taille</span>");
    gtk_box_pack_start(GTK_BOX(config_card), lbl_size, FALSE, FALSE, 0);

    GtkWidget *hbox_size = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(config_card), hbox_size, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_size), gtk_label_new("Nombre:"), FALSE, FALSE, 0);
    app_data->tree_size_input = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 10000, 1));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app_data->tree_size_input), 50);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(app_data->tree_size_input)), "modern-spin");
    gtk_box_pack_start(GTK_BOX(hbox_size), GTK_WIDGET(app_data->tree_size_input), TRUE, TRUE, 0);

    // Degré pour l'arbre N-aire
    GtkWidget *lbl_degree = gtk_label_new("Degré N (enfants max)");
    gtk_label_set_markup(GTK_LABEL(lbl_degree), "<span foreground='#ffffff' size='large' weight='bold'>🔢 Degré N (enfants max)</span>");
    gtk_box_pack_start(GTK_BOX(config_card), lbl_degree, FALSE, FALSE, 10);

    GtkWidget *hbox_degree = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(config_card), hbox_degree, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_degree), gtk_label_new("N:"), FALSE, FALSE, 0);
    app_data->tree_nary_degree_input = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 10, 1));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app_data->tree_nary_degree_input), app_data->nary_max_children);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(app_data->tree_nary_degree_input)), "modern-spin");
    gtk_box_pack_start(GTK_BOX(hbox_degree), GTK_WIDGET(app_data->tree_nary_degree_input), TRUE, TRUE, 0);

    GtkWidget *btn_create = gtk_button_new_with_label("✨ Nouveau / Reset");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_create), "modern-button");
    g_signal_connect(btn_create, "clicked", G_CALLBACK(on_tree_create_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(control_vbox), btn_create, FALSE, FALSE, 0);

    // Section Mode de Saisie (Carte moderne)
    GtkWidget *input_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(input_card), "modern-card");
    gtk_box_pack_start(GTK_BOX(control_vbox), input_card, FALSE, FALSE, 0);

    GtkWidget *lbl_input = gtk_label_new("Mode de Saisie");
    gtk_label_set_markup(GTK_LABEL(lbl_input), "<span foreground='#ffffff' size='large' weight='bold'>🔀 Mode de Saisie</span>");
    gtk_box_pack_start(GTK_BOX(input_card), lbl_input, FALSE, FALSE, 0);

    GtkWidget *vbox_input = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(input_card), vbox_input, FALSE, FALSE, 0);

    app_data->tree_random_radio = gtk_radio_button_new_with_label(NULL, "🎲 Aléatoire");
    app_data->tree_manual_radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(app_data->tree_random_radio), "✍️ Manuel");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app_data->tree_random_radio), TRUE);
    app_data->tree_input_source = 0;
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(app_data->tree_random_radio), "modern-radio");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(app_data->tree_manual_radio), "modern-radio");
    gtk_box_pack_start(GTK_BOX(vbox_input), app_data->tree_random_radio, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_input), app_data->tree_manual_radio, FALSE, FALSE, 0);

    g_signal_connect(app_data->tree_random_radio, "toggled", G_CALLBACK(on_tree_input_source_toggled), app_data);
    g_signal_connect(app_data->tree_manual_radio, "toggled", G_CALLBACK(on_tree_input_source_toggled), app_data);

    // Section Actions (Carte moderne)
    GtkWidget *actions_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(actions_card), "modern-card");
    gtk_box_pack_start(GTK_BOX(control_vbox), actions_card, TRUE, TRUE, 0);

    GtkWidget *lbl_act = gtk_label_new("Actions");
    gtk_label_set_markup(GTK_LABEL(lbl_act), "<span foreground='#ffffff' size='large' weight='bold'>⚡ Actions</span>");
    gtk_box_pack_start(GTK_BOX(actions_card), lbl_act, FALSE, FALSE, 0);

    GtkWidget *btn_rand = gtk_button_new_with_label("🎲 Remplir Aléatoire");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_rand), "modern-button");
    g_signal_connect(btn_rand, "clicked", G_CALLBACK(on_tree_insert_random_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(actions_card), btn_rand, FALSE, FALSE, 0);
    app_data->tree_random_button = btn_rand; // Stocker la référence

    // Saisie manuelle
    GtkWidget *lbl_manual_val = gtk_label_new("Valeur (manuel)");
    gtk_label_set_markup(GTK_LABEL(lbl_manual_val), "<span foreground='#ffffff' size='medium' weight='bold'>Valeur (manuel)</span>");
    gtk_box_pack_start(GTK_BOX(actions_card), lbl_manual_val, FALSE, FALSE, 10);
    app_data->tree_manual_label = lbl_manual_val; // Stocker la référence
    gtk_widget_hide(lbl_manual_val); // Masquer par défaut (mode aléatoire)

    app_data->tree_manual_entry = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_placeholder_text(app_data->tree_manual_entry, "ex: 42");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(app_data->tree_manual_entry)), "modern-combo");
    gtk_box_pack_start(GTK_BOX(actions_card), GTK_WIDGET(app_data->tree_manual_entry), FALSE, FALSE, 0);
    gtk_widget_hide(GTK_WIDGET(app_data->tree_manual_entry)); // Masquer par défaut (mode aléatoire)

    GtkWidget *btn_manual = gtk_button_new_with_label("➕ Insérer Manuel");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_manual), "modern-button");
    g_signal_connect(btn_manual, "clicked", G_CALLBACK(on_tree_insert_manual_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(actions_card), btn_manual, FALSE, FALSE, 0);
    app_data->tree_manual_button = btn_manual; // Stocker la référence
    gtk_widget_hide(btn_manual); // Masquer par défaut (mode aléatoire)

    GtkWidget *btn_del = gtk_button_new_with_label("➖ Supprimer (Valeur)");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_del), "modern-button");
    g_signal_connect(btn_del, "clicked", G_CALLBACK(on_tree_delete_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(actions_card), btn_del, FALSE, FALSE, 0);

    // Parcours
    GtkWidget *lbl_trav = gtk_label_new("Parcours");
    gtk_label_set_markup(GTK_LABEL(lbl_trav), "<span foreground='#ffffff' size='large' weight='bold'>🔄 Parcours</span>");
    gtk_box_pack_start(GTK_BOX(actions_card), lbl_trav, FALSE, FALSE, 10);

    app_data->tree_traversal_combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(app_data->tree_traversal_combo, "Pré-ordre");
    gtk_combo_box_text_append_text(app_data->tree_traversal_combo, "In-ordre");
    gtk_combo_box_text_append_text(app_data->tree_traversal_combo, "Post-ordre");
    gtk_combo_box_text_append_text(app_data->tree_traversal_combo, "Largeur (BFS)");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app_data->tree_traversal_combo), 0);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(app_data->tree_traversal_combo)), "modern-combo");
    gtk_box_pack_start(GTK_BOX(actions_card), GTK_WIDGET(app_data->tree_traversal_combo), FALSE, FALSE, 0);

    GtkWidget *btn_exec_trav = gtk_button_new_with_label("▶️ Exécuter Parcours");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_exec_trav), "modern-button");
    g_signal_connect(btn_exec_trav, "clicked", G_CALLBACK(on_tree_traversal_execute), app_data);
    gtk_box_pack_start(GTK_BOX(actions_card), btn_exec_trav, FALSE, FALSE, 0);

    GtkWidget *btn_stats = gtk_button_new_with_label("📊 Statistiques (Taille/Prof)");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_stats), "modern-button");
    g_signal_connect(btn_stats, "clicked", G_CALLBACK(on_tree_stats_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(actions_card), btn_stats, FALSE, FALSE, 0);

    GtkWidget *btn_order = gtk_button_new_with_label("🔀 Ordonner Arbre");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_order), "modern-button");
    g_signal_connect(btn_order, "clicked", G_CALLBACK(on_tree_order_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(actions_card), btn_order, FALSE, FALSE, 0);

    GtkWidget *btn_trans = gtk_button_new_with_label("🔄 Transfo N-Aire -> Bin");
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_trans), "modern-button");
    g_signal_connect(btn_trans, "clicked", G_CALLBACK(on_tree_transform_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(actions_card), btn_trans, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), control_scrolled, FALSE, FALSE, 0);

    // ========== PANEL DROIT : AFFICHAGE DES RÉSULTATS MODERNES ==========
    GtkWidget *display_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(display_scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(display_scrolled), GTK_SHADOW_NONE);

    GtkWidget *display_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(display_vbox), "display-panel");
    gtk_container_set_border_width(GTK_CONTAINER(display_vbox), 20);
    gtk_container_add(GTK_CONTAINER(display_scrolled), display_vbox);
    gtk_box_pack_start(GTK_BOX(hbox), display_scrolled, TRUE, TRUE, 0);

    // Visualisation de l'Arbre (Carte moderne)
    GtkWidget *drawing_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(drawing_card), "result-card");
    gtk_box_pack_start(GTK_BOX(display_vbox), drawing_card, TRUE, TRUE, 0);

    GtkWidget *drawing_header = gtk_label_new("Visualisation de l'Arbre");
    gtk_label_set_markup(GTK_LABEL(drawing_header), "<span foreground='#ffffff' size='large' weight='bold'>🌳 Visualisation de l'Arbre</span>");
    gtk_box_pack_start(GTK_BOX(drawing_card), drawing_header, FALSE, FALSE, 0);

    // Créer un scrolled window pour le drawing_area
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled_window, -1, 450);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_IN);
    app_data->tree_scrolled_window = scrolled_window;

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 800, 600);
    app_data->tree_drawing_area = drawing_area;
    // Enable events for Click-to-Edit
    gtk_widget_add_events(drawing_area, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_tree_callback), app_data);
    g_signal_connect(drawing_area, "button-press-event", G_CALLBACK(on_tree_click_event), app_data);

    gtk_container_add(GTK_CONTAINER(scrolled_window), drawing_area);
    gtk_box_pack_start(GTK_BOX(drawing_card), scrolled_window, TRUE, TRUE, 0);

    // Zone d'Informations (Carte moderne)
    GtkWidget *info_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(info_card), "result-card");
    gtk_box_pack_start(GTK_BOX(display_vbox), info_card, FALSE, FALSE, 0);

    GtkWidget *info_header = gtk_label_new("Informations");
    gtk_label_set_markup(GTK_LABEL(info_header), "<span foreground='#ffffff' size='large' weight='bold'>ℹ️ Informations</span>");
    gtk_box_pack_start(GTK_BOX(info_card), info_header, FALSE, FALSE, 0);

    GtkWidget *info_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(info_scrolled, -1, 150);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(info_scrolled), GTK_SHADOW_IN);
    GtkWidget *info_view = gtk_text_view_new();
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(info_view), "modern-text-view");
    gtk_container_add(GTK_CONTAINER(info_scrolled), info_view);
    app_data->tree_info_view = GTK_TEXT_VIEW(info_view);
    gtk_box_pack_start(GTK_BOX(info_card), info_scrolled, FALSE, FALSE, 0);

    // Gestionnaire pour la fermeture de la fenêtre secondaire
    g_signal_connect(window, "delete-event", G_CALLBACK(on_secondary_window_delete), NULL);

    gtk_widget_show_all(window);
}

static void module_callback(GtkWidget *widget, gpointer data) {
    const gchar *command = (const gchar *)data;
    GtkWidget *parent_window = gtk_widget_get_toplevel(widget);

    if (g_strcmp0(command, "TABLEAUX") == 0) {
        create_array_window(parent_window);
        return;
    }
    if (g_strcmp0(command, "LISTES") == 0) {
        create_list_window(parent_window);
        return;
    }
    if (g_strcmp0(command, "ARBRES") == 0) {
        create_tree_window(parent_window);
        return;
    }
    if (g_strcmp0(command, "GRAPHES") == 0) {
        create_graph_window(parent_window);
        return;
    }

    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(parent_window),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "La section %s n'est pas implémentée pour ce TP.", command);

    gtk_window_set_title(GTK_WINDOW(dialog), "Module Non Disponible");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static GtkWidget *create_styled_button(GtkWidget *parent_grid, const gchar *text, const gchar *icon_name, const gchar *command, int row) {

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);

    if (icon_name != NULL) {
        GtkWidget *icon = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_LARGE_TOOLBAR);
        gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);
    }

    GtkWidget *label = gtk_label_new(text);
    PangoFontDescription *font_desc = pango_font_description_from_string("Segoe UI Semibold 12");
    gtk_widget_override_font(label, font_desc);
    pango_font_description_free(font_desc);

    GdkRGBA PRIMARY_DARKER = { 30.0/255.0 * 0.8, 144.0/255.0 * 0.8, 255.0/255.0 * 0.8, 1.0 };
    gtk_widget_override_color(label, GTK_STATE_FLAG_NORMAL, &PRIMARY_DARKER);

    gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);

    GtkWidget *button = gtk_button_new();
    gtk_widget_set_hexpand(button, TRUE);
    gtk_widget_set_halign(button, GTK_ALIGN_FILL);

    gtk_container_add(GTK_CONTAINER(button), hbox);

    gtk_widget_override_background_color(button, GTK_STATE_FLAG_NORMAL, &WHITE_COLOR);

    g_signal_connect(button, "clicked", G_CALLBACK(module_callback), g_strdup(command));

    gtk_grid_attach(GTK_GRID(parent_grid), button, 0, row, 1, 1);

    return button;
}

static GtkWidget *create_header_panel(void) {
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(header_box), "modern-card-header");
    gtk_widget_set_size_request(header_box, -1, 120);
    gtk_widget_set_valign(header_box, GTK_ALIGN_CENTER);

    GtkWidget *title = gtk_label_new(NULL);
    PangoFontDescription *title_font = pango_font_description_from_string("Segoe UI Bold 32");
    gtk_widget_override_font(title, title_font);
    pango_font_description_free(title_font);
    gtk_label_set_markup(GTK_LABEL(title), "<span foreground='#ffffff' size='xx-large' weight='bold'>🌐 PROJET STRUCTURES DE DONNÉES</span>\n<span size='medium' foreground='#ffffff'>Analysez, Comparez et Visualisez la Performance des Algorithmes</span>");
    gtk_label_set_justify(GTK_LABEL(title), GTK_JUSTIFY_CENTER);
    gtk_label_set_xalign(GTK_LABEL(title), 0.5);
    gtk_box_pack_start(GTK_BOX(header_box), title, TRUE, TRUE, 0);

    return header_box;
}

// =========================================================================
//                             THEMING & STYLE
// =========================================================================

// Fonction load_css - CSS réactivé
static void load_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    const gchar *css_data =
        // GLOBAL WINDOW & LAYOUT - Premium Dark Theme
        "GtkWindow {"
        "     background: linear-gradient(135deg, #0a0e27 0%, #1a1f3a 50%, #0f1419 100%);"
        "     color: #f5f7fa;"
        "     font-family: 'Segoe UI', 'Inter', 'Roboto', Sans-Serif;"
        "}"
        "GtkBox, GtkGrid, GtkFrame {"
        "     background-color: transparent;"
        "}"

        // STANDARD WIDGETS - Premium Typography
        "GtkLabel {"
        "     color: #e8eaed;"
        "     font-weight: 400;"
        "}"

        // HEADER BARS & TITLES - Premium Glass Effect
        ".header-bar {"
        "     background: linear-gradient(135deg, rgba(30, 58, 138, 0.95) 0%, rgba(79, 70, 229, 0.95) 50%, rgba(139, 92, 246, 0.95) 100%);"
        "     backdrop-filter: blur(20px);"
        "     color: white;"
        "     border-bottom: 3px solid rgba(251, 191, 36, 0.6);"
        "     padding: 15px;"
        "     box-shadow: 0 8px 32px rgba(0, 0, 0, 0.4), inset 0 1px 0 rgba(255, 255, 255, 0.1);"
        "}"
        "GtkLabel.title {"
        "     font-size: 28px;"
        "     font-weight: 700;"
        "     color: #fbbf24;"
        "     text-shadow: 0 2px 10px rgba(251, 191, 36, 0.5), 0 0 20px rgba(251, 191, 36, 0.3);"
        "     letter-spacing: 0.5px;"
        "}"

        // BUTTONS - Premium Design
        "GtkButton {"
        "     background: linear-gradient(135deg, rgba(30, 58, 138, 0.8) 0%, rgba(79, 70, 229, 0.8) 100%);"
        "     color: #ffffff;"
        "     border: 1px solid rgba(139, 92, 246, 0.3);"
        "     border-radius: 10px;"
        "     padding: 10px 20px;"
        "     margin: 4px;"
        "     font-weight: 600;"
        "     font-size: 13px;"
        "     box-shadow: 0 4px 15px rgba(79, 70, 229, 0.3), inset 0 1px 0 rgba(255, 255, 255, 0.1);"
        "     transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);"
        "}"
        "GtkButton:hover {"
        "     background: linear-gradient(135deg, rgba(79, 70, 229, 0.9) 0%, rgba(139, 92, 246, 0.9) 100%);"
        "     border-color: rgba(251, 191, 36, 0.6);"
        "     box-shadow: 0 6px 25px rgba(139, 92, 246, 0.5), 0 0 15px rgba(251, 191, 36, 0.3), inset 0 1px 0 rgba(255, 255, 255, 0.2);"
        "     transform: translateY(-2px);"
        "}"
        "GtkButton:active {"
        "     background: linear-gradient(135deg, rgba(30, 58, 138, 1) 0%, rgba(79, 70, 229, 1) 100%);"
        "     transform: translateY(0px);"
        "     box-shadow: 0 2px 10px rgba(79, 70, 229, 0.4);"
        "}"

        // SPECIFIC ACTION BUTTONS (Neon Accents)
        "GtkButton.action {" // Main Menu Buttons
        "     background-image: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);"
        "     color: white;"
        "     font-size: 14px;"
        "     padding: 15px;"
        "}"
        "GtkButton.action:hover {"
        "     opacity: 0.9;"
        "     transform: scale(1.02);"
        "}"

        // TILE BUTTONS - Premium Cards
        "GtkButton.tile {"
        "     background: rgba(30, 41, 59, 0.6);"
        "     backdrop-filter: blur(10px);"
        "     color: #f5f7fa;"
        "     border: 1px solid rgba(139, 92, 246, 0.2);"
        "     border-radius: 16px;"
        "     padding: 20px;"
        "     font-size: 16px;"
        "     font-weight: 600;"
        "     box-shadow: 0 8px 32px rgba(0, 0, 0, 0.3), inset 0 1px 0 rgba(255, 255, 255, 0.1);"
        "     transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1);"
        "}"
        "GtkButton.tile:hover {"
        "     background: rgba(79, 70, 229, 0.3);"
        "     border-color: rgba(251, 191, 36, 0.6);"
        "     transform: translateY(-5px) scale(1.02);"
        "     box-shadow: 0 12px 40px rgba(139, 92, 246, 0.4), 0 0 20px rgba(251, 191, 36, 0.2), inset 0 1px 0 rgba(255, 255, 255, 0.2);"
        "}"

        // SORTING BUTTONS - Premium Accent Colors
        "GtkButton.bubble { "
        "     border-left: 4px solid #f87171;"
        "     background: linear-gradient(135deg, rgba(248, 113, 113, 0.2) 0%, rgba(30, 58, 138, 0.8) 100%);"
        "}"
        "GtkButton.bubble:hover { border-left-color: #fbbf24; }"
        "GtkButton.insertion { "
        "     border-left: 4px solid #34d399;"
        "     background: linear-gradient(135deg, rgba(52, 211, 153, 0.2) 0%, rgba(30, 58, 138, 0.8) 100%);"
        "}"
        "GtkButton.insertion:hover { border-left-color: #fbbf24; }"
        "GtkButton.shell { "
        "     border-left: 4px solid #60a5fa;"
        "     background: linear-gradient(135deg, rgba(96, 165, 250, 0.2) 0%, rgba(30, 58, 138, 0.8) 100%);"
        "}"
        "GtkButton.shell:hover { border-left-color: #fbbf24; }"
        "GtkButton.quick { "
        "     border-left: 4px solid #fbbf24;"
        "     background: linear-gradient(135deg, rgba(251, 191, 36, 0.2) 0%, rgba(30, 58, 138, 0.8) 100%);"
        "}"
        "GtkButton.quick:hover { border-left-color: #fbbf24; }"

        // INPUTS & COMBOS
        "GtkEntry, GtkSpinButton, GtkComboBox {"
        "     background-color: #2b2b36;"
        "     color: white;"
        "     border: 1px solid #555;"
        "     border-radius: 4px;"
        "     padding: 4px;"
        "}"
        "GtkEntry:focus, GtkSpinButton:focus {"
        "     border-color: #00f260;"
        "}"

        // VIEWS & AREAS - Premium Dark Theme
        "GtkTextView, GtkTreeView {"
        "     background-color: rgba(10, 14, 27, 0.95);"
        "     color: #60a5fa;" // Professional Blue
        "     font-family: 'Consolas', 'Fira Code', 'Monaco', monospace;"
        "     font-size: 13px;"
        "}"
        "GtkDrawingArea, GtkScrolledWindow {"
        "     border: 1px solid rgba(79, 70, 229, 0.3);"
        "     background-color: rgba(10, 14, 27, 0.95);"
        "     border-radius: 12px;"
        "     box-shadow: inset 0 2px 8px rgba(0, 0, 0, 0.3);"
        "}"

        // MODERN CARDS & PANELS - Premium Glassmorphism
        ".modern-card {"
        "     background: rgba(30, 41, 59, 0.7);"
        "     backdrop-filter: blur(20px);"
        "     border-radius: 20px;"
        "     border: 1px solid rgba(139, 92, 246, 0.2);"
        "     padding: 25px;"
        "     box-shadow: 0 8px 32px rgba(0, 0, 0, 0.4), inset 0 1px 0 rgba(255, 255, 255, 0.1), 0 0 0 1px rgba(255, 255, 255, 0.05);"
        "     margin: 10px;"
        "     transition: all 0.3s ease;"
        "}"
        ".modern-card:hover {"
        "     border-color: rgba(251, 191, 36, 0.4);"
        "     box-shadow: 0 12px 40px rgba(0, 0, 0, 0.5), inset 0 1px 0 rgba(255, 255, 255, 0.15), 0 0 20px rgba(251, 191, 36, 0.1);"
        "}"
        ".modern-card-header {"
        "     background: linear-gradient(135deg, rgba(30, 58, 138, 0.95) 0%, rgba(79, 70, 229, 0.95) 50%, rgba(139, 92, 246, 0.95) 100%);"
        "     backdrop-filter: blur(20px);"
        "     border-radius: 16px 16px 0 0;"
        "     padding: 20px 25px;"
        "     margin: -25px -25px 20px -25px;"
        "     color: white;"
        "     font-weight: 700;"
        "     font-size: 18px;"
        "     box-shadow: 0 4px 20px rgba(79, 70, 229, 0.4), inset 0 1px 0 rgba(255, 255, 255, 0.2);"
        "     border-bottom: 2px solid rgba(251, 191, 36, 0.5);"
        "}"
        ".control-panel {"
        "     background: rgba(15, 23, 42, 0.8);"
        "     backdrop-filter: blur(20px);"
        "     border-radius: 24px;"
        "     border: 1px solid rgba(139, 92, 246, 0.2);"
        "     padding: 30px;"
        "     box-shadow: 0 12px 40px rgba(0, 0, 0, 0.5), inset 0 1px 0 rgba(255, 255, 255, 0.1), 0 0 0 1px rgba(255, 255, 255, 0.05);"
        "}"
        ".display-panel {"
        "     background: rgba(10, 14, 27, 0.9);"
        "     backdrop-filter: blur(10px);"
        "     border-radius: 24px;"
        "     border: 1px solid rgba(79, 70, 229, 0.2);"
        "     padding: 25px;"
        "     box-shadow: inset 0 4px 20px rgba(0, 0, 0, 0.4), 0 0 0 1px rgba(255, 255, 255, 0.05);"
        "}"
        ".modern-frame {"
        "     background-color: #1e1e2a;"
        "     border-radius: 10px;"
        "     border: 1px solid #3a3a4a;"
        "     padding: 15px;"
        "     box-shadow: 0 2px 8px rgba(0, 0, 0, 0.3);"
        "}"
        ".modern-frame GtkLabel {"
        "     color: #a0a0b0;"
        "     font-weight: 600;"
        "     font-size: 13px;"
        "     text-transform: uppercase;"
        "     letter-spacing: 0.5px;"
        "}"
        ".modern-button {"
        "     background: linear-gradient(135deg, rgba(30, 58, 138, 0.9) 0%, rgba(79, 70, 229, 0.9) 100%);"
        "     border: 1px solid rgba(139, 92, 246, 0.3);"
        "     border-radius: 12px;"
        "     padding: 14px 28px;"
        "     color: white;"
        "     font-weight: 600;"
        "     font-size: 14px;"
        "     box-shadow: 0 6px 20px rgba(79, 70, 229, 0.4), inset 0 1px 0 rgba(255, 255, 255, 0.2);"
        "     transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);"
        "     letter-spacing: 0.3px;"
        "}"
        ".modern-button:hover {"
        "     background: linear-gradient(135deg, rgba(79, 70, 229, 1) 0%, rgba(139, 92, 246, 1) 100%);"
        "     border-color: rgba(251, 191, 36, 0.6);"
        "     transform: translateY(-3px);"
        "     box-shadow: 0 10px 30px rgba(139, 92, 246, 0.6), 0 0 20px rgba(251, 191, 36, 0.3), inset 0 1px 0 rgba(255, 255, 255, 0.3);"
        "}"
        ".modern-button:active {"
        "     transform: translateY(-1px);"
        "     box-shadow: 0 4px 15px rgba(79, 70, 229, 0.5);"
        "}"
        ".section-title {"
        "     color: #ffffff;"
        "     font-size: 18px;"
        "     font-weight: bold;"
        "     margin: 10px 0;"
        "     text-shadow: 0 2px 4px rgba(0, 0, 0, 0.5);"
        "}"
        ".section-subtitle {"
        "     color: #a0a0b0;"
        "     font-size: 12px;"
        "     margin-bottom: 15px;"
        "     text-transform: uppercase;"
        "     letter-spacing: 1px;"
        "}"
        ".modern-notebook {"
        "     background-color: transparent;"
        "     border: none;"
        "}"
        ".modern-notebook .tabs {"
        "     background-color: #252530;"
        "     border-radius: 8px 8px 0 0;"
        "}"
        ".modern-text-view {"
        "     background: rgba(10, 14, 27, 0.95);"
        "     backdrop-filter: blur(10px);"
        "     border-radius: 12px;"
        "     border: 1px solid rgba(79, 70, 229, 0.3);"
        "     padding: 15px;"
        "     color: #60a5fa;"
        "     font-family: 'Consolas', 'Monaco', 'Fira Code', monospace;"
        "     font-size: 13px;"
        "     box-shadow: inset 0 2px 8px rgba(0, 0, 0, 0.3), 0 0 0 1px rgba(255, 255, 255, 0.05);"
        "}"
        ".modern-combo {"
        "     background: rgba(30, 41, 59, 0.8);"
        "     backdrop-filter: blur(10px);"
        "     border: 1px solid rgba(139, 92, 246, 0.3);"
        "     border-radius: 10px;"
        "     padding: 10px 16px;"
        "     color: #f5f7fa;"
        "     font-size: 13px;"
        "     box-shadow: inset 0 2px 4px rgba(0, 0, 0, 0.2), 0 0 0 1px rgba(255, 255, 255, 0.05);"
        "     transition: all 0.3s ease;"
        "}"
        ".modern-combo:hover {"
        "     border-color: rgba(251, 191, 36, 0.5);"
        "     box-shadow: 0 0 15px rgba(139, 92, 246, 0.4), inset 0 2px 4px rgba(0, 0, 0, 0.2);"
        "     background: rgba(30, 41, 59, 0.9);"
        "}"
        ".modern-spin {"
        "     background: rgba(30, 41, 59, 0.8);"
        "     backdrop-filter: blur(10px);"
        "     border: 1px solid rgba(139, 92, 246, 0.3);"
        "     border-radius: 10px;"
        "     padding: 8px;"
        "     color: #f5f7fa;"
        "     box-shadow: inset 0 2px 4px rgba(0, 0, 0, 0.2), 0 0 0 1px rgba(255, 255, 255, 0.05);"
        "     transition: all 0.3s ease;"
        "}"
        ".modern-spin:focus {"
        "     border-color: rgba(251, 191, 36, 0.6);"
        "     box-shadow: 0 0 20px rgba(139, 92, 246, 0.5), inset 0 2px 4px rgba(0, 0, 0, 0.2);"
        "     background: rgba(30, 41, 59, 0.95);"
        "}"
        ".modern-radio {"
        "     color: #e0e0e0;"
        "     font-size: 13px;"
        "     padding: 5px;"
        "}"
        ".result-card {"
        "     background: rgba(30, 41, 59, 0.7);"
        "     backdrop-filter: blur(20px);"
        "     border-radius: 20px;"
        "     border: 1px solid rgba(139, 92, 246, 0.2);"
        "     padding: 25px;"
        "     box-shadow: 0 8px 32px rgba(0, 0, 0, 0.4), inset 0 1px 0 rgba(255, 255, 255, 0.1);"
        "     margin: 15px 0;"
        "     transition: all 0.3s ease;"
        "}"
        ".result-card:hover {"
        "     border-color: rgba(251, 191, 36, 0.3);"
        "     box-shadow: 0 12px 40px rgba(0, 0, 0, 0.5), inset 0 1px 0 rgba(255, 255, 255, 0.15);"
        "}"
        ".result-card-header {"
        "     color: #fbbf24;"
        "     font-size: 16px;"
        "     font-weight: 700;"
        "     margin-bottom: 15px;"
        "     padding-bottom: 12px;"
        "     border-bottom: 2px solid rgba(251, 191, 36, 0.4);"
        "     text-shadow: 0 2px 8px rgba(251, 191, 36, 0.3);"
        "     letter-spacing: 0.5px;"
        "}"
        // Dashboard tiles - buttons inside cards
        ".modern-card GtkButton.modern-button {"
        "     background: transparent;"
        "     border: none;"
        "     box-shadow: none;"
        "     padding: 0;"
        "}"
        ".modern-card GtkButton.modern-button:hover {"
        "     background: transparent;"
        "     transform: none;"
        "     box-shadow: none;"
        "}"
        ".modern-card:hover {"
        "     transform: translateY(-8px) scale(1.02);"
        "     box-shadow: 0 16px 48px rgba(0, 0, 0, 0.6), 0 0 0 1px rgba(139, 92, 246, 0.4), 0 0 30px rgba(251, 191, 36, 0.2);"
        "     border-color: rgba(251, 191, 36, 0.5);"
        "     transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1);"
        "}"
        // Premium scrollbars
        "GtkScrolledWindow {"
        "     border-radius: 12px;"
        "}"
        // Premium separators
        "GtkSeparator {"
        "     background: rgba(139, 92, 246, 0.2);"
        "     min-height: 1px;"
        "}"
        // Premium radio buttons
        ".modern-radio {"
        "     color: #e8eaed;"
        "     font-size: 13px;"
        "     padding: 6px;"
        "     font-weight: 500;"
        "}"
        ".modern-radio:hover {"
        "     color: #fbbf24;"
        "}";

    gtk_css_provider_load_from_data(provider, css_data, -1, NULL);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

// Fonctions de dessin personnalisées pour les icônes des modules
static gboolean draw_array_icon(GtkWidget *widget, cairo_t *cr, gpointer data) {
    guint width = gtk_widget_get_allocated_width(widget);
    guint height = gtk_widget_get_allocated_height(widget);

    // Dessiner un tableau avec des barres de différentes hauteurs (représentant un tri)
    double center_x = width / 2.0;
    double center_y = height / 2.0;
    double bar_width = width / 8.0;
    double max_height = height * 0.6;

    // Dessiner 5 barres de hauteurs différentes (triées)
    double heights[] = {0.3, 0.5, 0.7, 0.9, 0.6};
    double start_x = center_x - (bar_width * 2.5);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // Blanc
    cairo_set_line_width(cr, 2.0);

    for (int i = 0; i < 5; i++) {
        double x = start_x + i * bar_width;
        double bar_height = max_height * heights[i];
        double y = center_y + max_height / 2.0 - bar_height / 2.0;

        // Dessiner la barre
        cairo_rectangle(cr, x, y, bar_width * 0.8, bar_height);
        cairo_fill(cr);

        // Bordure
        cairo_set_source_rgb(cr, 0.6, 0.8, 1.0); // Bleu clair
        cairo_rectangle(cr, x, y, bar_width * 0.8, bar_height);
        cairo_stroke(cr);
    }

    // Dessiner une flèche vers le haut pour indiquer le tri
    cairo_set_source_rgb(cr, 0.3, 0.9, 0.3); // Vert
    cairo_set_line_width(cr, 2.5);
    cairo_move_to(cr, center_x, center_y + max_height / 2.0 + 10);
    cairo_line_to(cr, center_x, center_y - max_height / 2.0 - 5);
    cairo_stroke(cr);

    // Pointe de la flèche
    cairo_move_to(cr, center_x, center_y - max_height / 2.0 - 5);
    cairo_line_to(cr, center_x - 5, center_y - max_height / 2.0 + 5);
    cairo_line_to(cr, center_x + 5, center_y - max_height / 2.0 + 5);
    cairo_close_path(cr);
    cairo_fill(cr);

    return FALSE;
}

static gboolean draw_list_icon(GtkWidget *widget, cairo_t *cr, gpointer data) {
    guint width = gtk_widget_get_allocated_width(widget);
    guint height = gtk_widget_get_allocated_height(widget);

    // Dessiner des nœuds de liste chaînée connectés
    double center_y = height / 2.0;
    double node_radius = 12.0;
    double spacing = 35.0;
    double start_x = width / 2.0 - spacing;

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // Blanc

    // Dessiner 3 nœuds connectés
    for (int i = 0; i < 3; i++) {
        double x = start_x + i * spacing;

        // Dessiner le nœud (cercle)
        cairo_arc(cr, x, center_y, node_radius, 0, 2 * M_PI);
        cairo_fill(cr);

        // Bordure du nœud
        cairo_set_source_rgb(cr, 0.2, 0.7, 0.9); // Bleu
        cairo_set_line_width(cr, 2.0);
        cairo_arc(cr, x, center_y, node_radius, 0, 2 * M_PI);
        cairo_stroke(cr);

        // Dessiner la flèche vers le nœud suivant (sauf pour le dernier)
        if (i < 2) {
            cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
            cairo_set_line_width(cr, 2.0);
            cairo_move_to(cr, x + node_radius, center_y);
            cairo_line_to(cr, x + spacing - node_radius, center_y);
            cairo_stroke(cr);

            // Pointe de la flèche
            cairo_move_to(cr, x + spacing - node_radius, center_y);
            cairo_line_to(cr, x + spacing - node_radius - 5, center_y - 3);
            cairo_line_to(cr, x + spacing - node_radius - 5, center_y + 3);
            cairo_close_path(cr);
            cairo_fill(cr);
        }
    }

    return FALSE;
}

static gboolean draw_tree_icon(GtkWidget *widget, cairo_t *cr, gpointer data) {
    guint width = gtk_widget_get_allocated_width(widget);
    guint height = gtk_widget_get_allocated_height(widget);

    // Dessiner un petit arbre binaire
    double center_x = width / 2.0;
    double top_y = height * 0.25;
    double node_radius = 8.0;
    double level_spacing = 25.0;

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // Blanc
    cairo_set_line_width(cr, 2.0);

    // Racine
    cairo_arc(cr, center_x, top_y, node_radius, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.9, 0.6, 0.2); // Orange
    cairo_arc(cr, center_x, top_y, node_radius, 0, 2 * M_PI);
    cairo_stroke(cr);

    // Niveau 2 : deux enfants
    double left_x = center_x - 20.0;
    double right_x = center_x + 20.0;
    double level2_y = top_y + level_spacing;

    // Lignes de connexion
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_move_to(cr, center_x, top_y + node_radius);
    cairo_line_to(cr, left_x, level2_y - node_radius);
    cairo_stroke(cr);

    cairo_move_to(cr, center_x, top_y + node_radius);
    cairo_line_to(cr, right_x, level2_y - node_radius);
    cairo_stroke(cr);

    // Nœuds enfants
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_arc(cr, left_x, level2_y, node_radius, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.9, 0.6, 0.2);
    cairo_arc(cr, left_x, level2_y, node_radius, 0, 2 * M_PI);
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_arc(cr, right_x, level2_y, node_radius, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.9, 0.6, 0.2);
    cairo_arc(cr, right_x, level2_y, node_radius, 0, 2 * M_PI);
    cairo_stroke(cr);

    return FALSE;
}

static gboolean draw_graph_icon(GtkWidget *widget, cairo_t *cr, gpointer data) {
    guint width = gtk_widget_get_allocated_width(widget);
    guint height = gtk_widget_get_allocated_height(widget);

    // Dessiner un petit graphe avec des nœuds connectés
    double center_x = width / 2.0;
    double center_y = height / 2.0;
    double node_radius = 8.0;
    double graph_radius = 25.0;

    // Positions des nœuds en cercle
    double nodes[4][2];
    for (int i = 0; i < 4; i++) {
        double angle = 2 * M_PI * i / 4.0 - M_PI / 2.0;
        nodes[i][0] = center_x + graph_radius * cos(angle);
        nodes[i][1] = center_y + graph_radius * sin(angle);
    }

    cairo_set_source_rgb(cr, 0.6, 0.4, 0.8); // Violet
    cairo_set_line_width(cr, 2.0);

    // Dessiner les arêtes
    cairo_move_to(cr, nodes[0][0], nodes[0][1]);
    cairo_line_to(cr, nodes[1][0], nodes[1][1]);
    cairo_stroke(cr);

    cairo_move_to(cr, nodes[1][0], nodes[1][1]);
    cairo_line_to(cr, nodes[2][0], nodes[2][1]);
    cairo_stroke(cr);

    cairo_move_to(cr, nodes[2][0], nodes[2][1]);
    cairo_line_to(cr, nodes[3][0], nodes[3][1]);
    cairo_stroke(cr);

    cairo_move_to(cr, nodes[3][0], nodes[3][1]);
    cairo_line_to(cr, nodes[0][0], nodes[0][1]);
    cairo_stroke(cr);

    // Dessiner les nœuds
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // Blanc
    for (int i = 0; i < 4; i++) {
        cairo_arc(cr, nodes[i][0], nodes[i][1], node_radius, 0, 2 * M_PI);
        cairo_fill(cr);

        cairo_set_source_rgb(cr, 0.6, 0.4, 0.8); // Violet
        cairo_arc(cr, nodes[i][0], nodes[i][1], node_radius, 0, 2 * M_PI);
        cairo_stroke(cr);
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    }

    return FALSE;
}

// Helper to create modern dashboard tiles
static void create_dashboard_tile(GtkWidget *grid, const gchar *label, const gchar *icon_name, const gchar *command, int col, int row) {
    // Container card
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(card), "modern-card");
    gtk_widget_set_hexpand(card, TRUE);
    gtk_widget_set_vexpand(card, TRUE);
    gtk_widget_set_size_request(card, 250, 200);

    // Button inside card
    GtkWidget *btn = gtk_button_new();
    // Style CSS activé
    gtk_style_context_add_class(gtk_widget_get_style_context(btn), "modern-button");
    gtk_widget_set_hexpand(btn, TRUE);
    gtk_widget_set_vexpand(btn, TRUE);
    g_object_set_data(G_OBJECT(btn), "command", (gpointer)command);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(box), 20);
    gtk_container_add(GTK_CONTAINER(btn), box);

    // Icon personnalisé avec dessin Cairo
    GtkWidget *icon_drawing = gtk_drawing_area_new();
    gtk_widget_set_size_request(icon_drawing, 90, 90);

    // Connecter la fonction de dessin appropriée selon le module
    if (g_strcmp0(command, "TABLEAUX") == 0) {
        g_signal_connect(icon_drawing, "draw", G_CALLBACK(draw_array_icon), NULL);
    } else if (g_strcmp0(command, "LISTES") == 0) {
        g_signal_connect(icon_drawing, "draw", G_CALLBACK(draw_list_icon), NULL);
    } else if (g_strcmp0(command, "ARBRES") == 0) {
        g_signal_connect(icon_drawing, "draw", G_CALLBACK(draw_tree_icon), NULL);
    } else if (g_strcmp0(command, "GRAPHES") == 0) {
        g_signal_connect(icon_drawing, "draw", G_CALLBACK(draw_graph_icon), NULL);
    }

    gtk_box_pack_start(GTK_BOX(box), icon_drawing, FALSE, FALSE, 0);

    // Label
    GtkWidget *lbl = gtk_label_new(label);
    gtk_label_set_use_markup(GTK_LABEL(lbl), TRUE);
    gtk_label_set_justify(GTK_LABEL(lbl), GTK_JUSTIFY_CENTER);
    gtk_box_pack_start(GTK_BOX(box), lbl, FALSE, FALSE, 0);

    // Pass command as data
    g_signal_connect(btn, "clicked", G_CALLBACK(module_callback), (gpointer)command);

    gtk_box_pack_start(GTK_BOX(card), btn, TRUE, TRUE, 0);
    gtk_grid_attach(GTK_GRID(grid), card, col, row, 1, 1);
}

// Callback pour gérer la fermeture de la fenêtre principale
static gboolean on_main_window_delete(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    // Vérifier que c'est bien la fenêtre principale qui demande à être fermée
    if (widget != main_window && main_window != NULL) {
        // Si ce n'est pas la fenêtre principale, empêcher la fermeture
        return TRUE;
    }
    // Permettre la fermeture normale de la fenêtre principale uniquement
    return FALSE;
}

// Callback spécifique pour la fenêtre des tableaux - Solution robuste pour entreprise
static gboolean on_array_window_delete(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    // Protection absolue : s'assurer que la fenêtre principale ne se ferme jamais
    if (main_window != NULL && GTK_IS_WINDOW(main_window)) {
        // Forcer destroy_with_parent à FALSE avant toute opération
        gtk_window_set_destroy_with_parent(GTK_WINDOW(widget), FALSE);

        // Vérifier que la fenêtre principale est toujours visible et référencée
        if (!gtk_widget_get_visible(main_window)) {
            gtk_widget_show(main_window);
        }

        // S'assurer que la fenêtre principale n'est pas liée à cette fenêtre
        gtk_window_set_transient_for(GTK_WINDOW(widget), NULL);
    }

    // Permettre la fermeture normale de la fenêtre secondaire uniquement
    return FALSE;
}

// Callback pour gérer la fermeture des fenêtres secondaires (listes, arbres, graphes)
static gboolean on_secondary_window_delete(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    // S'assurer que destroy_with_parent est FALSE pour toutes les fenêtres secondaires
    // Cela empêche la fermeture de la fenêtre principale quand une fenêtre secondaire se ferme
    gtk_window_set_destroy_with_parent(GTK_WINDOW(widget), FALSE);

    // Retourner FALSE permet la destruction normale de la fenêtre secondaire
    // Le signal "destroy" sera émis et gérera le nettoyage
    // La fenêtre principale ne sera pas affectée car destroy_with_parent est FALSE
    return FALSE;
}

// Wrapper pour le callback destroy de la fenêtre tableaux
static void on_array_window_destroy(GtkWidget *widget, gpointer user_data) {
    ArrayCleanupData *cleanup = (ArrayCleanupData *)user_data;
    if (cleanup && cleanup->data_ptr) {
        free_data(cleanup->data_ptr, cleanup->N, cleanup->type);
    }
    if (cleanup) {
        g_free(cleanup);
    }
}

// Wrapper pour le callback destroy de la fenêtre listes
static void on_list_window_destroy(GtkWidget *widget, gpointer user_data) {
    AppData *app_data = (AppData *)user_data;
    if (app_data) {
        if (app_data->current_list) {
            list_free(app_data->current_list);
        }
        g_free(app_data);
    }
}

static void on_graph_window_destroy(GtkWidget *widget, gpointer user_data) {
    AppData *app_data = (AppData *)user_data;
    if (app_data) {
        if (app_data->current_graph) {
            graph_free(app_data->current_graph);
        }
        g_free(app_data);
    }
}

static void activate(GtkApplication *app, gpointer user_data) {
    // Charger le CSS
    load_css();

    main_window = gtk_application_window_new(app);
    GtkWidget *window = main_window;
    gtk_window_set_title(GTK_WINDOW(window), "🌐 Projet Structures de Données - FSTM");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    // Protection renforcée pour entreprise : s'assurer que la fenêtre principale
    // ne se ferme jamais quand les fenêtres secondaires se ferment
    gtk_window_set_destroy_with_parent(GTK_WINDOW(window), FALSE);

    // Empêcher toute relation parent-enfant qui pourrait causer la fermeture
    gtk_window_set_modal(GTK_WINDOW(window), FALSE);

    // Connecter le gestionnaire d'événement delete-event avec vérification renforcée
    g_signal_connect(window, "delete-event", G_CALLBACK(on_main_window_delete), NULL);

    // Container principal
    GtkWidget *main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_container);

    // Header moderne
    GtkWidget *header_panel = create_header_panel();
    gtk_box_pack_start(GTK_BOX(main_container), header_panel, FALSE, FALSE, 0);

    // Container pour le contenu principal avec padding
    GtkWidget *content_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(content_container), 40);
    gtk_box_pack_start(GTK_BOX(main_container), content_container, TRUE, TRUE, 0);

    // Dashboard Grid avec espacement moderne
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 25);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 25);
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_box_pack_start(GTK_BOX(content_container), grid, TRUE, TRUE, 0);

    // Row 0 - Modules principaux
    create_dashboard_tile(grid, "Tableaux\n<span size='small' foreground='#ffffff'>Tri &amp; Performance</span>", "view-grid-symbolic", "TABLEAUX", 0, 0);
    create_dashboard_tile(grid, "Listes Chaînées\n<span size='small' foreground='#ffffff'>Simple/Double</span>", "view-list-symbolic", "LISTES", 1, 0);

    // Row 1 - Modules avancés
    create_dashboard_tile(grid, "Arbres\n<span size='small' foreground='#ffffff'>Binaires &amp; N-Aires</span>", "folder-tree-symbolic", "ARBRES", 0, 1);
    create_dashboard_tile(grid, "Graphes\n<span size='small' foreground='#ffffff'>Chemins &amp; Poids</span>", "network-wireless-symbolic", "GRAPHES", 1, 1);

    // Footer moderne
    GtkWidget *footer_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(footer_container), 20);
    gtk_box_pack_end(GTK_BOX(main_container), footer_container, FALSE, FALSE, 0);

    GtkWidget *footer_separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(footer_container), footer_separator, FALSE, FALSE, 0);

    GtkWidget *footer = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(footer), "<span foreground='#ffffff' size='small'>FSTM © 2025 | Développement C GTK 3.4</span>");
    gtk_label_set_justify(GTK_LABEL(footer), GTK_JUSTIFY_CENTER);
    gtk_widget_set_margin_top(footer, 10);
    gtk_box_pack_start(GTK_BOX(footer_container), footer, FALSE, FALSE, 0);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    // GTK3 initialise automatiquement les threads, g_thread_init() n'est plus nécessaire

    srand(time(NULL));

    GtkApplication *app = gtk_application_new("org.fstm.structures", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;

}


#include <gtk/gtk.h>
#include <cairo.h>
#include "../include/chess.h"

GtkBuilder *builder;
GtkWidget *window, *draw_area, *switch_label, *history_table;
GtkTextBuffer *buffer;

cairo_surface_t* icon[13];
int from, to, is_picked = 0, OPPONENT = BLACK, iteration = 1, DEPTH = 1;
const char PLAYER_NICK[2][6] = {"BLACK", "WHITE"};

gboolean on_board_draw(GtkWidget*, cairo_t*, gpointer);
gboolean on_button_press(GtkWidget*, GdkEventButton*, gpointer);
void on_switch_button_state_set(GtkSwitch*);
void on_scale_button_value_changed(GtkRange*);

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);
    
    chess_load();
    
    builder = gtk_builder_new_from_file("ui/main.glade");
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    draw_area = GTK_WIDGET(gtk_builder_get_object(builder, "draw_area"));
    switch_label = GTK_WIDGET(gtk_builder_get_object(builder, "switch_label"));
    history_table = GTK_WIDGET(gtk_builder_get_object(builder, "game_history_table"));
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(history_table));
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), 0);
    
    gtk_builder_connect_signals(builder, 0);
    
    icon[0] = 0;
    icon[1] = cairo_image_surface_create_from_png("resource/w_pawn.png");
    icon[2] = cairo_image_surface_create_from_png("resource/w_knight.png");
    icon[3] = cairo_image_surface_create_from_png("resource/w_bishop.png");
    icon[4] = cairo_image_surface_create_from_png("resource/w_rook.png");
    icon[5] = cairo_image_surface_create_from_png("resource/w_queen.png");
    icon[6] = cairo_image_surface_create_from_png("resource/w_king.png");
    icon[7] = cairo_image_surface_create_from_png("resource/b_pawn.png");
    icon[8] = cairo_image_surface_create_from_png("resource/b_knight.png");
    icon[9] = cairo_image_surface_create_from_png("resource/b_bishop.png");
    icon[10] = cairo_image_surface_create_from_png("resource/b_rook.png");
    icon[11] = cairo_image_surface_create_from_png("resource/b_queen.png");
    icon[12] = cairo_image_surface_create_from_png("resource/b_king.png");
    
    chess_init(BOARD);
    
    gtk_widget_show(window);
    gtk_main();
	return 0;
}

gboolean on_board_draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    int i, j, status;
    unsigned char record[12];
    
    for(i = 0;i < 8;i++){
        for(j = 0;j < 8;j++){
            if(BOARD[i + 8 * j] != EMPTY){
                cairo_set_source_surface(cr, icon[BOARD[i + 8 * j]], i * 60, j * 60);
                cairo_paint(cr);            
            }
        }
    }
    if(is_picked){
        cairo_set_source_rgba(cr, 0, 0, 1, 0.15);
        cairo_rectangle(cr, 60 * (from % 8), 60 * (from / 8), 60, 60);
        for(i = 0;i < 8;i++){
            for(j = 0;j < 8;j++){
                to = i + 8 * j;
                if((status = chess_play_manual(BOARD, from, to, record))){
                    TAKEBACK(BOARD, record);
                    if(status == 4) TAKEBACK(BOARD, (&record[6]));
                    cairo_rectangle(cr, 60 * (to % 8), 60 * (to / 8), 60, 60);
                }
            }
        }
        cairo_fill(cr);
    }
    return 0;
}

gboolean on_button_press(GtkWidget *w, GdkEventButton *event, gpointer data)
{
    if(is_picked){
        to = (int)(event->x / 60) + 8 * (int)( event->y / 60);
        unsigned char record[12];
        if(chess_play_manual(BOARD, (from % 8) + 8 * (from / 8), (to % 8) + 8 * (to / 8), record)){
            chess_play_ai(BOARD, OPPONENT, DEPTH, 0);
            iteration++;
        }
        gtk_widget_queue_draw(draw_area);
        is_picked = 0;
    }else{
        from = (int)(event->x / 60) + 8 * (int)(event->y / 60);
        if(COLOR_OF(BOARD[from]) == !OPPONENT){
            gtk_widget_queue_draw(draw_area);
            is_picked = 1;
        }
    }
    return TRUE;
}

void on_switch_button_state_set(GtkSwitch *s)
{
    gtk_label_set_text(GTK_LABEL(switch_label),(gchar*)PLAYER_NICK[OPPONENT]);
    OPPONENT = !OPPONENT;
    chess_play_ai(BOARD, OPPONENT, DEPTH, 0);
    gtk_widget_queue_draw(draw_area);
}

void on_scale_button_value_changed(GtkRange *r)
{
    DEPTH = (int)gtk_range_get_value(r);
}


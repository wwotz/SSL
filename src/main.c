#include <stdio.h>
#include <sys/time.h>

#include "../include/ui.h"
#include "../include/shader.h"

static GLuint shader_pid;
static GLfloat *shader_vertices;
static GLuint vertex_buffer;
static GLuint *shader_indices;
static GLuint index_buffer;

static GLfloat iTime;
static GLuint iTime_uniform;

static int iResolution[2];
static GLuint iResolution_uniform;

static struct timeval start_time, curr_time;

static void shader_setup(const char *, const char *);
static void shader_render(void);
static void shader_free(void);

int main(int argc, char **argv)
{
	ui_init(UI_WINDOW_NAME, UI_WINDOW_WIDTH, UI_WINDOW_HEIGHT, UI_WINDOW_FLAGS);

	shader_setup("./shaders/shader.glvs", "./shaders/shader.glfs");

	SDL_Event event;
	while (ui_active) {
		while (SDL_PollEvent(&event)) {
			ui_events(event);
		}
		ui_render_start();
		shader_render();
		ui_render_end();
	}

	shader_free();
	ui_free();
	return printf("SSL - Closed\n");
}


static void shader_setup(const char *vertex_path, const char *fragment_path)
{
 	shader_vertices = (GLfloat *)malloc(12 * sizeof *shader_vertices);
	shader_indices = (GLuint *)malloc(6 * sizeof *shader_indices);

	*(shader_vertices+0) = 0.f;
	*(shader_vertices+1) = 0.f;
	*(shader_vertices+2) = 1.f;
	*(shader_vertices+3) = 0.f;
	*(shader_vertices+4) = 1.f;
	*(shader_vertices+5) = 1.f;
	*(shader_vertices+6) = 1.f;
	*(shader_vertices+7) = 1.f;
	*(shader_vertices+8) = 0.f;
	*(shader_vertices+9) = 1.f;
	*(shader_vertices+10) = 0.f;
	*(shader_vertices+11) = 0.f;

	for (int i = 0; i < 6; i++) {
		*(shader_indices+i) = i;
	}

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof *shader_vertices, shader_vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof *shader_indices, shader_indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	shader_pid = glCreateProgram();
	GLuint vertex_shader = shader_load(vertex_path, GL_VERTEX_SHADER);
	if (0 == vertex_shader) {
		glDeleteProgram(shader_pid);
		shader_pid = 0;
		return;
	}
	glAttachShader(shader_pid, vertex_shader);

	GLuint fragment_shader = shader_load(fragment_path, GL_FRAGMENT_SHADER);
	if (0 == fragment_shader) {
		glDeleteShader(vertex_shader);
		glDeleteProgram(shader_pid);
		shader_pid = 0;
		return;
	}
	glAttachShader(shader_pid, fragment_shader);

	glLinkProgram(shader_pid);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	iTime_uniform = glGetUniformLocation(shader_pid, "iTime");
	if (-1 == iTime_uniform) {
		printf("iTime is not a valid glsl program variable\n");
	}

	iTime = 0.f;
	gettimeofday(&start_time, 0);

	/* Custom iResolution uniform, soon to be implemented
	iResolution_uniform = glGetUniformLocation(shader_pid, "iResolution");
	if (-1 == iResolution_uniform) {
		printf("iResolution is not a valid glsl program variable\n");
	}

	glUseProgram(shader_pid);
	
	SDL_GetWindowSize(window, iResolution+0, iResolution+1);
	glUniform2iv(iResolution_uniform, 1, iResolution);
	printf("Width: %d, Height: %d\n", *(iResolution+0), *(iResolution+1));

	glUseProgram(0);
	*/
}

static void shader_render(void)
{
	glUseProgram(shader_pid);
	glLoadIdentity();
	glScalef(UI_WINDOW_WIDTH, UI_WINDOW_HEIGHT, 1.0);
	
	glUniform1f(iTime_uniform, iTime);

	glEnableClientState(GL_VERTEX_ARRAY);
	
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glVertexPointer(2, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

	glDisableClientState(GL_VERTEX_ARRAY);
	
	glUseProgram(0);
	gettimeofday(&curr_time, 0);
	iTime = (curr_time.tv_sec - start_time.tv_sec) + (curr_time.tv_usec - start_time.tv_usec)/1000000.f;
}

static void shader_free(void)
{
	glDeleteProgram(shader_pid);
	free(shader_vertices);
	free(shader_indices);
	shader_pid = 0;
}

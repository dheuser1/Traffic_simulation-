#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include "View.h"
#include <GL/glew.h>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <math.h>
#include <iostream>
#include <chrono>
#include <random>
#include <fstream>
#include <string>
#include <sstream>
#include "car.h"

using namespace std;
#include "OBJImporter.h"
#include "OBJExporter.h"

//lead car takes longer to start moving than the rest in a jam

//glm headers to access various matrix producing functions, like ortho below in resize
#include <glm/gtc/matrix_transform.hpp>
//the glm header required to convert glm objects into normal float/int pointers expected by OpenGL
//see value_ptr function below for an example
#include <glm/gtc/type_ptr.hpp>

const double c=88.0/60.0;

View::View()
{
	trackballTransform = glm::mat4(1.0);
	fill=true;
	made_file=false;
	cout<<"You will be asked to enter data about the simulation " <<endl;
	cout<<"note that the number of cars * space between cars must be less than 600"<<endl;
	cout<<"a time step of about .01 is recomended"<<endl;

	cout<<"enter the number of cars, both roads will have the same number of cars"<<endl;
	cin>>num_cars;
	cout<<"enter the space between cars in feet"<<endl;
	cin>>car_spacing;
	cout<<"enter length of simulation in seconds, may not be how long program will run"<<endl;
	cin>>sim_time;
	cout<<"enter in seconds, how long it takes the light to change"<<endl;
	cin>>light_time;
	cout<<"enter the length of the time step in seconds"<<endl;
	cin>>time_step;
	num_run=sim_time/time_step;
	fill_cars();
	//use same vector for both roads but have 2 car_num ints to split them in drawing, will need to chnage breaking function a bit, 
	//part about head car needsto change

	count=0;
	light_1=70;
	light_2=220;
	light_state=true;
	light_count=0;
}

View::~View()
{
	for (int i=0;i<objectsList.size();i++)
	{
		delete objectsList[i];
	}

	objectsList.clear();

	for(int i=0; i<cars.size(); i++)
	{
		delete cars[i];
	}
	cars.clear();
}

void View::resize(int w, int h)
{
	//record the new dimensions
	WINDOW_WIDTH = w;
	WINDOW_HEIGHT = h;

	/*
	* This program uses orthographic projection. The corresponding matrix for this projection is provided by the glm function below.
	*The last two parameters are for the near and far planes.
	*
	*Very important: the last two parameters specify the position of the near and far planes with respect
	*to the eye, in the direction of gaze. Thus positive values are in front of the camera, and negative
	*values are in the back!
	**/

	while (!proj.empty())
		proj.pop();

	proj.push(glm::ortho(-200.0f,200.0f,-200.0f*WINDOW_HEIGHT/WINDOW_WIDTH,200.0f*WINDOW_HEIGHT/WINDOW_WIDTH,0.1f,10000.0f));
	// proj.push(glm::perspective(120.0f*3.14159f/180,(float)WINDOW_WIDTH/WINDOW_HEIGHT,0.1f,10000.0f));
}
//this will set up the roads and cars to show on screen
void View::initialize()
{
	double PI = 3.14159;
	//populate our shader information. The two files below are present in this project.
	ShaderInfo shaders[] =
	{
		{GL_VERTEX_SHADER,"triangles.vert"},
		{GL_FRAGMENT_SHADER,"triangles.frag"},
		{GL_NONE,""} //used to detect the end of this array
	};

	//call helper function, get the program shader ID if everything went ok.
	program = createShaders(shaders);

	//use the above program. After this statement, any rendering will use this above program
	//passing 0 to the function below disables using any shaders
	glUseProgram(program);

	//assuming the program above is compiled and linked correctly, get IDs for all the input variables
	//that the shader expects our program to provide.
	//think of these IDs as references to those shader variables

	//the second parameter of glGetUniformLocation is a string that is the name of an actual variable in the shader program
	//this variable may exist in any of the shaders that are linked in this program.

	projectionLocation = glGetUniformLocation(program,"projection");
	modelviewLocation = glGetUniformLocation(program,"modelview");
	objectColorLocation = glGetAttribLocation(program,"vColor");

	Object * o;
	TriangleMesh m;

	OBJImporter::importFile(m,string("models/box"),false);

	//the 1st road
	o = new Object();
	o->init(m);
	o->setColor(1,1,1);
	o->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(0,75,-4)) * glm::scale(glm::mat4(1.0),glm::vec3(8,150,1)));
	objectsList.push_back(o);

	o = new Object();
	o->init(m);
	o->setColor(1,1,1);
	o->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(150,75,-4)) * glm::scale(glm::mat4(1.0),glm::vec3(8,150,1)));
	objectsList.push_back(o);

	o = new Object();
	o->init(m);
	o->setColor(1,1,1);
	o->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(75,0,-4)) * glm::scale(glm::mat4(1.0),glm::vec3(158,8,1)));
	objectsList.push_back(o);

	o = new Object();
	o->init(m);
	o->setColor(1,1,1);
	o->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(75,150,-4)) * glm::scale(glm::mat4(1.0),glm::vec3(158,8,1)));
	objectsList.push_back(o);

	//the second road
	o = new Object();
	o->init(m);
	o->setColor(0,1,1);
	o->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(0,77,-4)) * glm::scale(glm::mat4(1.0),glm::vec3(150,8,1)));
	objectsList.push_back(o);

	o = new Object();
	o->init(m);
	o->setColor(0,1,1);
	o->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(75,2,-4)) * glm::scale(glm::mat4(1.0),glm::vec3(8,158,1)));
	objectsList.push_back(o);

	o = new Object();
	o->init(m);
	o->setColor(0,1,1);
	o->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(-75,2,-4)) * glm::scale(glm::mat4(1.0),glm::vec3(8,158,1)));
	objectsList.push_back(o);

	o = new Object();
	o->init(m);
	o->setColor(0,1,1);
	o->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(0,-73,-4)) * glm::scale(glm::mat4(1.0),glm::vec3(150,8,1)));
	objectsList.push_back(o);

	o = new Object();
	o->init(m);
	o->setColor(1,0,0);
	o->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(12,70,0)) * glm::scale(glm::mat4(1.0),glm::vec3(8,2,10)));
	objectsList.push_back(o);

	o = new Object();
	o->init(m);
	o->setColor(0,1,0);
	o->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(-7,65,0)) * glm::scale(glm::mat4(1.0),glm::vec3(2,8,10)));
	objectsList.push_back(o);

	//the cars
	for(int i=0; i<num_cars*2; i++)
	{
		o = new Object();
		o->init(m);
		o->setColor(1,0,0);
		o->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(0,0,0)) * glm::scale(glm::mat4(1.0),glm::vec3(5,5,5)));
		objectsList.push_back(o);
	}
	glUseProgram(0);
}

void View::draw()
{
	//do a time step of the simulation
	if(count<num_run)
	{
		simulate();
		count++;
		light_count++;
		if(light_count*time_step>light_time)
		{
			//draw the light the right color
			light_count=0;
			if(light_state==true)
			{
				light_state=false;
				objectsList[9]->setColor(1,0,0);
				objectsList[8]->setColor(0,1,0);
			}
			else
			{
				light_state=true;
				objectsList[9]->setColor(0,1,0);
				objectsList[8]->setColor(1,0,0);
			}
		}
	}
	else if(made_file==false)
	{
		made_file=true;
		to_file();
		cout<<"Done"<<endl;
	}

	float PI = 3.14159;
	/*
	*The modelview matrix for the View class is going to store the world-to-view transformation
	*This effectively is the transformation that changes when the camera parameters chang
	*This matrix is provided by glm::lookAt
	*/
	glUseProgram(program);
	while (!modelview.empty())
		modelview.pop();

	modelview.push(glm::mat4(1.0));
	modelview.top() = modelview.top() * glm::lookAt(glm::vec3(0,0,200),glm::vec3(0,0,0),glm::vec3(0,1,0))*trackballTransform;

	//center roads
	modelview.top() = modelview.top()* glm::translate(glm::mat4(1.0),glm::vec3(-50,-50,0));


	glUniformMatrix4fv(projectionLocation,1,GL_FALSE,glm::value_ptr(proj.top()));
	/*
	*Instead of directly supplying the modelview matrix to the shader here, we pass it to the objects
	*This is because the object's transform will be multiplied to it before it is sent to the shader
	*for vertices of that object only.
	*
	*Since every object is in control of its own color, we also pass it the ID of the color
	*in the activated shader program.
	*
	*This is so that the objects can supply some of their attributes without having any direct control
	*of the shader itself.
	*/
	//change this to fill to make solid 
	if(fill==false)
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	}

	//move car and draw it in the right place
	for (int i=0;i<objectsList.size();i++)
	{
		if(i>9 && i<num_cars+10)
		{
			int pos=fmod(cars[i-10]->get_position(),600.0);
			objectsList[i]->setColor(1,0,0);
			//make laed car diffrent color
			if(i==10)
			{
				objectsList[i]->setColor(0,1,0);
			}
			
			if(pos<150)
			{
				objectsList[i]->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(-3,pos,0)) * glm::scale(glm::mat4(1.0),glm::vec3(3,5,5)));
			}
			else if(pos>150 && pos<300)
			{
				objectsList[i]->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(pos-150,153,0)) * glm::scale(glm::mat4(1.0),glm::vec3(5,3,5)));
			}
			else if(pos>300 && pos<450)
			{
				objectsList[i]->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(153,450-pos,0)) * glm::scale(glm::mat4(1.0),glm::vec3(3,5,5)));
			}
			else if(pos>450 && pos<600)
			{
				objectsList[i]->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(600-pos,-3,0)) * glm::scale(glm::mat4(1.0),glm::vec3(5,3,5)));
			}
		}
		if(i>9 && i>=num_cars+10)
		{
			int pos=fmod(cars[i-10]->get_position(),600.0);
			objectsList[i]->setColor(1,0,0);
			//make laed car diffrent color
			if(i==10+num_cars)
			{
				objectsList[i]->setColor(0,1,0);
			}
			if(pos<150)
			{
				objectsList[i]->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(-78,pos-75,0)) * glm::scale(glm::mat4(1.0),glm::vec3(3,5,5)));
			}
			else if(pos>150 && pos<300)
			{
				objectsList[i]->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(pos-225,80,0)) * glm::scale(glm::mat4(1.0),glm::vec3(5,3,5)));
			}
			else if(pos>300 && pos<450)
			{
				objectsList[i]->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(78,375-pos,0)) * glm::scale(glm::mat4(1.0),glm::vec3(3,5,5)));
			}
			else if(pos>450 && pos<600)
			{
				objectsList[i]->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(525-pos,-77,0)) * glm::scale(glm::mat4(1.0),glm::vec3(5,3,5)));
			}
		}
		
		glm::mat4 transform = objectsList[i]->getTransform();
		glm::vec4 color = objectsList[i]->getColor();
		//The total transformation is whatever was passed to it, with its own transformation
		glUniformMatrix4fv(modelviewLocation,1,GL_FALSE,glm::value_ptr(modelview.top() * transform));
		//set the color for all vertices to be drawn for this object
		glVertexAttrib3fv(objectColorLocation,glm::value_ptr(color));
		objectsList[i]->draw();
	}

	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glFinish();
	modelview.pop();
	glUseProgram(0);
}

//if w is pressed togle wireframe on and off
void View::set_fill()
{
	fill=(fill==true)?false:true;
}

void View::clear()
{
	trackballTransform = glm::mat4(1.0);
}

void View::mousepress(int x, int y)
{
	prev_mouse = glm::vec2(x,y);
}

void View::mousemove(int x, int y)
{
	int dx,dy;

	dx = x - prev_mouse.x;
	dy = (y) - prev_mouse.y;

	if ((dx==0) && (dy==0))
		return;

	//(-dy,dx) gives the axis of rotation

	//the angle of rotation is calculated in radians by assuming that the radius of the trackball is 300
	float angle = sqrt((float)dx*dx+dy*dy)/100;

	prev_mouse = glm::vec2(x,y);

	trackballTransform = glm::rotate(glm::mat4(1.0),angle,glm::vec3(-dy,dx,0)) * trackballTransform;
}

/*
*This is a helper function that will take shaders info as a parameter, compiles them and links them
*into a shader program.
*
*This function is standard and should not change from one program to the next.
*/

GLuint View::createShaders(ShaderInfo *shaders)
{
	ifstream file;
	GLuint shaderProgram;
	GLint linked;

	ShaderInfo *entries = shaders;

	shaderProgram = glCreateProgram();


	while (entries->type !=GL_NONE)
	{
		file.open(entries->filename.c_str());
		GLint compiled;


		if (!file.is_open())
			return false;

		string source,line;


		getline(file,line);
		while (!file.eof())
		{
			source = source + "\n" + line;
			getline(file,line);
		}
		file.close();


		const char *codev = source.c_str();


		entries->shader = glCreateShader(entries->type);
		glShaderSource(entries->shader,1,&codev,NULL);
		glCompileShader(entries->shader);
		glGetShaderiv(entries->shader,GL_COMPILE_STATUS,&compiled);

		if (!compiled)
		{
			printShaderInfoLog(entries->shader);
			for (ShaderInfo *processed = shaders;processed->type!=GL_NONE;processed++)
			{
				glDeleteShader(processed->shader);
				processed->shader = 0;
			}
			return 0;
		}
		glAttachShader( shaderProgram, entries->shader );
		entries++;
	}

	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram,GL_LINK_STATUS,&linked);

	if (!linked)
	{
		printShaderInfoLog(entries->shader);
		for (ShaderInfo *processed = shaders;processed->type!=GL_NONE;processed++)
		{
			glDeleteShader(processed->shader);
			processed->shader = 0;
		}
		return 0;
	}

	return shaderProgram;
}

void View::printShaderInfoLog(GLuint shader)
{
	int infologLen = 0;
	int charsWritten = 0;
	GLubyte *infoLog;

	glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&infologLen);
	//	printOpenGLError();
	if (infologLen>0)
	{
		infoLog = (GLubyte *)malloc(infologLen);
		if (infoLog != NULL)
		{
			glGetShaderInfoLog(shader,infologLen,&charsWritten,(char *)infoLog);
			printf("InfoLog: %s\n\n",infoLog);
			free(infoLog);
		}

	}
	//	printOpenGLError();
}

void View::getOpenGLVersion(int *major,int *minor)
{
	const char *verstr = (const char *)glGetString(GL_VERSION);
	if ((verstr == NULL) || (sscanf_s(verstr,"%d.%d",major,minor)!=2))
	{
		*major = *minor = 0;
	}
}

void View::getGLSLVersion(int *major,int *minor)
{
	int gl_major,gl_minor;

	getOpenGLVersion(&gl_major,&gl_minor);
	*major = *minor = 0;

	if (gl_major==1)
	{
		/* GL v1.x can only provide GLSL v1.00 as an extension */
		const char *extstr = (const char *)glGetString(GL_EXTENSIONS);
		if ((extstr!=NULL) && (strstr(extstr,"GL_ARB_shading_language_100")!=NULL))
		{
			*major = 1;
			*minor = 0;
		}
	}
	else if (gl_major>=2)
	{
		/* GL v2.0 and greater must parse the version string */
		const char *verstr = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
		if ((verstr==NULL) || (sscanf_s(verstr,"%d.%d",major,minor) !=2))
		{
			*major = 0;
			*minor = 0;
		}
	}
}

//use normal distribution for velocity and fill teh array of cars
void View::fill_cars()
{
	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	default_random_engine generator(seed);
	normal_distribution<double> distribution(30.0,1.0);
	//first road
	for(int i=0; i<(num_cars); i++)
	{
		cars.push_back(new car(distribution(generator),0,(num_cars-1)*car_spacing-i*car_spacing, 0));
	}
	//second road
	for(int i=0; i<(num_cars); i++)
	{
		cars.push_back(new car(distribution(generator),0,(num_cars-1)*car_spacing-i*car_spacing, 0));
	}
}
//hold data to be put in file
void View::make_data(const int i, const double v, const double p, const double d, const bool b, const int v_size, const string state)
{
	stringstream s;
	// car# v, x. distance
	s<<i<<" vel= "<<v<<" pos= "<<p<<" dis= "<<d<<" bre= "<<b<<" sta= "<<state<<endl;	
	data.push_back(s.str());
	s.str("");
	//make data easier to read
	if(i==num_cars-1)
	{
		data.push_back("***********************************\n");
	}
	else if(i==cars.size()-1)
	{
		data.push_back("-----------------------------------\n");
	}
}
//save the data in a file
void View::to_file()
{
	ofstream outstream;
	outstream.open("road_data.txt");
	if(outstream.fail())
	{
		cout<<"opening failed"<<endl;
		exit(1);
	}

	for(int i=0; i<data.size(); i++)
	{
		outstream<<data[i];
	}
	outstream.close();
}
//run a step of the simulation
void View::simulate()
{
	double distance=0;
	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	default_random_engine generator(seed);
	normal_distribution<double> distribution(0.0,.1);
	
	breaking(distribution, generator);
}
//see how the car breaks
void View::breaking(normal_distribution<double> &distribution, default_random_engine &generator)
{
	string state="default";
	double distance=0.0;

	for(int i=0; i<num_cars*2; i++)
	{
		bool crawl=true;
		int temp=i-1;
		//first car looks at last car
		if(temp==-1)
		{
			temp=num_cars-1;
		}
		else if(temp==num_cars-1)
		{
			temp=cars.size()-1;
		}
		
		int pos=fmod(cars[i]->get_position(),600.0);
		//if only one car it should never slow down for another car, but should for the light so make distance really big
		if(num_cars==1)
		{
			distance=1000;
		}
		else if(i==0)
		{
			//need to do the first car on its own
			distance=(cars[0]->get_position())-(cars[num_cars-1]->get_position());
			distance=600-distance;
		}
		else if(i==num_cars)
		{
			//need to do the first car on its own
			distance=(cars[num_cars]->get_position())-(cars[cars.size()-1]->get_position());
			distance=600-distance;
		}
		else
		{
			//need to undo distance car ahead went
			distance=(cars[i-1]->get_position()-c*cars[i-1]->get_velocity()*time_step)-cars[i]->get_position();
		}
		//this is for breaking
		if(distance<10.0 || (i<num_cars && (light_1-pos)<10.0 && (light_1-pos)>0 && light_state==true) 
		   ||(i>=num_cars && (light_2-pos)<10.0  && (light_2-pos)>0 && light_state==false))
		{
			//too close car stops as velocity is 0 position will stay the same
			state="full stop";
			make_data(i, cars[i]->get_velocity(), cars[i]->get_position(), distance, cars[i]->get_break_light(), cars.size(), state);
			cars[i]->set_velocity(0.0);
			cars[i]->set_break_light(1);
			crawl=false;	
		}
		else if(distance<25.0 || (i<num_cars && (light_1-pos)<25 && (light_1-pos)>0 && light_state==true) 
			    ||(i>=num_cars && (light_2-pos)<25 && (light_2-pos)>0 && light_state==false))
		{
			//slow down by 90% per second
			state="slow 90%";
			make_data(i, cars[i]->get_velocity(), cars[i]->get_position(), distance, cars[i]->get_break_light(), cars.size(), state);
			cars[i]->set_velocity((cars[i]->get_velocity())-.9*time_step*cars[i]->get_velocity());
			cars[i]->set_break_light(1);
			cars[i]->set_position(c*cars[i]->get_velocity()*time_step+cars[i]->get_position());
		}
		//only slow down if car ahead of you is going slower 
		else if((distance<45.0 && cars[temp]->get_velocity()<cars[i]->get_velocity()) 
			   ||(i<num_cars && (light_1-pos)<45 &&(light_1-pos)>0 && light_state==true) 
			   ||(i>=num_cars && (light_2-pos)<45 &&(light_2-pos)>0 && light_state==false))
		{
			//slow down by 70% per second
			state="slow 70%";
			make_data(i, cars[i]->get_velocity(), cars[i]->get_position(), distance, cars[i]->get_break_light(), cars.size(), state);
			cars[i]->set_velocity((cars[i]->get_velocity())-.7*time_step*cars[i]->get_velocity());
			cars[i]->set_break_light(1);
			cars[i]->set_position(c*cars[i]->get_velocity()*time_step+cars[i]->get_position());
		}
		else if((distance<65.0 && cars[i]->get_break_light()!=0 && cars[temp]->get_velocity()<cars[i]->get_velocity()))
		{
			//slow down by 35% per second
			state="slow 35%";
			make_data(i, cars[i]->get_velocity(), cars[i]->get_position(), distance, cars[i]->get_break_light(), cars.size(), state);
			cars[i]->set_velocity((cars[i]->get_velocity())-.35*time_step*cars[i]->get_velocity());
			cars[i]->set_break_light(1);
			cars[i]->set_position(c*cars[i]->get_velocity()*time_step+cars[i]->get_position());
		}
		else if((distance<65.0 && cars[i]->get_break_light()==0 && cars[temp]->get_velocity()<cars[i]->get_velocity())
			    ||(i<num_cars && (light_1-pos)<65.0 && (light_1-pos)>0 && light_state==true) 
				||(i>=num_cars && (light_2-pos)<65.0 && (light_2-pos)>0 && light_state==false))
		{
			//slow down by 10% per second, foot off gas but not on break so break light is off
			state="slow 10%";
			make_data(i, cars[i]->get_velocity(), cars[i]->get_position(), distance, cars[i]->get_break_light(), cars.size(), state);
			cars[i]->set_velocity((cars[i]->get_velocity())-.1*time_step*cars[i]->get_velocity());
			cars[i]->set_break_light(0);
			cars[i]->set_position(c*cars[i]->get_velocity()*time_step+cars[i]->get_position());
		}
		else if(cars[i]->get_velocity()<30.0)
		{
			//speed up by 10mph per second
			state="speed up 10mph";
			make_data(i, cars[i]->get_velocity(), cars[i]->get_position(), distance, cars[i]->get_break_light(), cars.size(), state);
			cars[i]->set_velocity((cars[i]->get_velocity()+10.0*time_step));
			cars[i]->set_break_light(0);
			cars[i]->set_position(c*cars[i]->get_velocity()*time_step+cars[i]->get_position());
			crawl=false;
		}
		else
		{
			state="steady";
			make_data(i, cars[i]->get_velocity(), cars[i]->get_position(), distance, cars[i]->get_break_light(), cars.size(), state);
			cars[i]->set_velocity(cars[i]->get_velocity()+distribution(generator));
			cars[i]->set_position(c*cars[i]->get_velocity()*time_step+cars[i]->get_position());
			crawl=false;
		}
		//want to stop deadlock
		if((crawl==true && cars[temp]->get_velocity()<.5 && cars[i]->get_velocity()<3.0) ||
		   (crawl==true && cars[temp]->get_velocity()<.5 && cars[i]->get_velocity()<.5))
		{
			state="crawl";
			//undo movement
			cars[i]->set_position(cars[i]->get_position()-c*cars[i]->get_velocity()*time_step);
			data.pop_back();
			make_data(i, cars[i]->get_velocity(), cars[i]->get_position(), distance, cars[i]->get_break_light(), cars.size(), state);
			cars[i]->set_velocity(2.8);
			cars[i]->set_break_light(1);
			cars[i]->set_position(c*cars[i]->get_velocity()*time_step+cars[i]->get_position());
		}
	}
}
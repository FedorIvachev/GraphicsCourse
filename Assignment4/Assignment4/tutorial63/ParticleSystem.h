#include <stdlib.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define PI 3.14159265359

class SnowFlake
{
    glm::vec3 gravAcceleratio;
	glm::vec3 initialSpeed;
	double initialLife;
	glm::vec3 force;
	glm::vec3 speed;
	double mass;
	double life;

public:
	double scale = 10.0f;
	glm::vec3 position;
	glm::vec4 color;

public:
	SnowFlake(double m, glm::vec3 positionValue, glm::vec3 speedValue,
		glm::vec3 gravity, double lifeValue, double scaleValue)
	{
		gravAcceleratio = gravity;
		position = positionValue;
		speed = speedValue;
		initialSpeed = speedValue;
		scale = scaleValue;
		color = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
        life = lifeValue;
		initialLife = lifeValue;
		mass = m;
		force = glm::vec3(0.0, 0.0, 0.0);
	}

	~SnowFlake()
	{
	}

	void apply()
	{
		force += gravAcceleratio * GLfloat(mass);
	}

	void update(double dt, glm::vec3 newPos)
	{
		
		speed += force / GLfloat(mass) * GLfloat(dt);
		position += speed * GLfloat(dt);


		life -= dt;
		if (life < 5) {
			color.a = 0.5 * life / 5;
		}
		if (life < 1e-2) // revive
		{
			speed = initialSpeed;
            color = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
			position = newPos;
			life = initialLife;
		}
	}

	void process(double dt, glm::vec3 newpos)
	{
		force = glm::vec3(0.0, 0.0, 0.0);
		apply(); // apply force (add force)
		update(dt, newpos); // update speed and position
	}

};

void jump_particles(Player *player)
{
	for (int i = 0; i < 15; i++)
	{
		Particle p = {};
		p.position = player->body_id->position;
		p.lifetime = 1.5f;
		p.from_color = V4(0.59f, 0.43f, 0.25f, 0.5f);
		p.to_color = V4(1, 1, 1, 0);
		p.angular_velocity = random_real_in_range(&rnd, 0.0f, 1.0f);
		p.linear_velocity = random_vec2(&rnd, V2(-0.8f, 0.0f), V2(0.8f, 0.6f));
		p.scale = V2(0.7f, 0.7f);
		add_particle(&particle_system, p);
	}
}

void shoot_particles(Player *player)
{
	for (int i = 0; i < 20; i++)
	{
		Particle p = {};
		p.lifetime = 1;
		p.scale = V2(0.2f, 0.2f);
		p.position = player->body_id->position;
		p.position.x += player->face_direction * 0.5f;

		p.to_color = V4(0, 0, 0, 0);
		switch(player->weapon)
		{
			case JELLO: 
				p.from_color = V4(0, 1, 0, 0.5f);
				break;
			case CARROT:
				p.from_color = V4(0.98f, 0.61f, 0.12f, 0.5f);
				break;
			case ONION: 
				p.from_color = V4(0.8f, 0.6f, 0, 0.5f);
				break;
		}

		p.angular_velocity = random_real_in_range(&rnd, -1.0f, 1.0f);
		p.linear_velocity = random_vec2(&rnd, V2(-0.5f, -0.5f), V2(0.5f, 0.5f));
		add_particle(&particle_system, p);
	}
}

void movement_particles(Player *player)
{
	Particle p = {};
	p.position = player->body_id->position;
	p.lifetime = 2;
	p.from_color = V4(0.59f, 0.43f, 0.25f, 0.5f);
	p.to_color = V4(1, 1, 1, 0);
	p.angular_velocity = random_real_in_range(&rnd, 0.0f, 1.0f);
	p.linear_velocity = random_vec2(&rnd, V2(-0.0f, 0.0f), V2(0.8f, 0.6f));
	p.scale = V2(0.7f, 0.7f);
	add_particle(&particle_system, p);
}

void bounce_particles(Player *player)
{
	Particle p = {};
	p.position = player->body_id->position + V2(random_real_in_range(&rnd, -0.5f, 0.5f), 0);
	p.lifetime = 2;
	p.from_color = V4(0, 1, 0, 1);
	p.to_color = V4(0, 1, 0, 1);
	p.linear_velocity = V2(0, 0);
	p.scale = V2(0.1f, 0.1f);
	p.gravity = GRAVITY/150.0f;
	add_particle(&particle_system, p);
}

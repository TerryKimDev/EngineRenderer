#include "dev_app.h"
//#include "debug_renderer.h"
#include <iostream>
//TODO include debug_renderer.h and pools.h and anything else you might need here
#include "pools.h"
namespace end
{

	struct particle {
		float3 pos;
		float4 clr;
		float3 vel;
		float3 prev_pos;
	};

	struct emitter
	{
		float3 spawn_pos;
		float4 spawn_color;
		// indices into the shared_pool 
		sorted_pool_t<uint16_t, 256> indices;
	};


	view_t v;
	frustum_t frust;
	emitter yellowemit = { float3{4,0,4}, float4{1,1,0,1} };
	emitter Bluemilk = { float3{-4,0,4}, float4{0,0,1,1} };
	emitter RedSchmit = { float3{4,0,-4}, float4{1,0,0,1} };
	emitter GreenRanger = { float3{-4,0,-4}, float4{0,1,0,1} };
	double delta_time = 0.0;
	double red = 0, green = 0, blue = 0;
	

	float4x4 Lookat = (float4x4&)(DirectX::XMMatrixIdentity());
	DirectX::XMMATRIX lm = (DirectX::XMMATRIX&)Lookat;

	float4x4 at = (float4x4&)(DirectX::XMMatrixIdentity());

	float4x4 rotater = (float4x4&)(DirectX::XMMatrixIdentity());
	
	const float maxSpeed = 8.f;

	sorted_pool_t<particle, 256> pool;

	pool_t<particle, 1024> free;
	aabb_t box1 = { {3,4,3}, {1,2,3} };

	float grv = -9.8f;
	float randomFloat(float min, float max) {
		float ratio = rand() / (float)RAND_MAX;
		return (max - min) * ratio + min;
	}


	void DrawAxis(float4x4 m) {

		float3 start = m[3].xyz;
		float3 endx = start + m[0].xyz * 1;
		float3 endy = start + m[1].xyz * 1;
		float3 endz = start + m[2].xyz * 1;
		end::debug_renderer::add_line(start, endx, { 1,0,0,1 });
		end::debug_renderer::add_line(start, endy, { 0,1,0,1 });
		end::debug_renderer::add_line(start, endz, { 0,0,1,1 });

	}

	DirectX::XMMATRIX lookatTarget(float3 eye, float3 tar, float3 up) {
		//float3 eyexyz = eye[3].xyz;
		//float3 atxyz = at[3].xyz;

		float3 colm3 = colm3.normalize(tar - eye);
		float3 colm1 = colm1.normalize(colm1.cross(up, colm3));
		float3 colm2 = colm2.normalize(colm2.cross(colm3, colm1));


		DirectX::XMMATRIX temp = DirectX::XMMatrixIdentity();
		temp.r[0] = { colm1.x, colm2.x, colm3.x, 0 };
		temp.r[1] = { colm1.y, colm2.y, colm3.y, 0 };
		temp.r[2] = { colm1.z, colm2.z, colm3.z, 0 };

		float3 xpos;
		float3 ypos;
		float3 zpos;
		temp.r[3] = { xpos.dot((colm1 * -1), eye), xpos.dot((colm2 * -1), eye),xpos.dot((colm3 * -1), eye), 1 };

		temp = DirectX::XMMatrixInverse(nullptr, temp);

		return temp;
		//line to check where it is looking

	}

	void turntoTarget(float4x4& viewer, float4x4 target, float speed) {
		DirectX::XMMATRIX rm = (DirectX::XMMATRIX&)viewer;
		float3 viewerxyz = viewer[3].xyz;
		float3 targetxyz = target[3].xyz;
		
		float3 v = v.normalize(targetxyz - viewerxyz);
		float yturnrate = v.dot(v, viewer[0].xyz);

		float4x4 masy = (float4x4&)(DirectX::XMMatrixRotationY(yturnrate * speed * delta_time));

		float xturnrate = v.dot(v, viewer[1].xyz);
		float4x4 masx = (float4x4&)(DirectX::XMMatrixRotationX(-xturnrate * speed * delta_time));

		viewer = (float4x4&)(DirectX::XMMatrixMultiply((DirectX::XMMATRIX&)masy, rm));
		viewer = (float4x4&)(DirectX::XMMatrixMultiply((DirectX::XMMATRIX&)masx, (DirectX::XMMATRIX&)viewer));
		viewer = (float4x4&)lookatTarget(viewer[3].xyz, viewer[3].xyz + viewer[2].xyz, {0,1,0});
	}

	void orthoMat(float4x4_a& cam) {
		float3 zaxis = zaxis.normalize(cam[2].xyz);
		float3 xaxis = zaxis.normalize(xaxis.cross({ 0,1,0 }, zaxis));
		float3 yaxis = yaxis.normalize(yaxis.cross(zaxis, xaxis));

		cam[0].xyz = xaxis;
		cam[1].xyz = yaxis;
		cam[2].xyz = zaxis;
	}



	void dev_app_t::mouselook(float4x4_a& cam) {


		if (mouse[0] == true)
		{
			if (mx_prev != mx)
			{
				float4x4 masx = (float4x4&)(DirectX::XMMatrixRotationY(DirectX::XMConvertToDegrees((mx_prev - mx) * delta_time * .02)));
				cam = (float4x4_a&)(DirectX::XMMatrixMultiply((DirectX::XMMATRIX&)masx, (DirectX::XMMATRIX&)cam));

			}
			if (my_prev != my)
			{
				float4x4 masy = (float4x4&)(DirectX::XMMatrixRotationX(DirectX::XMConvertToDegrees((my_prev - my ) * delta_time * .02)));
				cam = (float4x4_a&)(DirectX::XMMatrixMultiply((DirectX::XMMATRIX&)masy, (DirectX::XMMATRIX&)cam));
			}
			
		}

		orthoMat(cam);
		mx_prev = mx;
		my_prev = my;
	}

	
	void dev_app_t::keypressing() {
		float perframespeed = maxSpeed * delta_time;

		if (keypress['W'] == true) {

			at[3].xyz += at[2].xyz * perframespeed;
		}
		if (keypress['S'] == true)
		{
			at[3].xyz -= at[2].xyz * perframespeed;
		}
		if (keypress['A'] == true)
		{
			float4x4 temp = (float4x4&)(DirectX::XMMatrixRotationY(-perframespeed));
			DirectX::XMMATRIX am = (DirectX::XMMATRIX&)at;
			at = (float4x4&)(DirectX::XMMatrixMultiply((DirectX::XMMATRIX&)temp, am));
		}
		if (keypress['D'] == true)
		{
			float4x4 temp = (float4x4&)(DirectX::XMMatrixRotationY(perframespeed));
			DirectX::XMMATRIX am = (DirectX::XMMATRIX&)at;
			at = (float4x4&)(DirectX::XMMatrixMultiply((DirectX::XMMATRIX&)temp, am));
		}
		if (keypress[16] == true)//lshift
		{
			at[3].y -= perframespeed;
		}
		if (keypress[32] == true) //space
		{
			at[3].y += perframespeed;
		}
		
	}

	void dev_app_t::worldmove(float4x4_a& cam) {

		float perframespeed = maxSpeed * delta_time;
		if (keypress[38] == true) //up
		{
			cam[3].xyz += cam[2].xyz * perframespeed;
		}
		if (keypress[37] == true) //left
		{
			float4x4 temp = (float4x4&)(DirectX::XMMatrixRotationY(-perframespeed*.2));
			DirectX::XMMATRIX am = (DirectX::XMMATRIX&)cam;
			cam = (float4x4_a&)(DirectX::XMMatrixMultiply((DirectX::XMMATRIX&)temp, am));
		}
		if (keypress[39] == true) //right
		{
			float4x4 temp = (float4x4&)(DirectX::XMMatrixRotationY(perframespeed *.2));
			DirectX::XMMATRIX am = (DirectX::XMMATRIX&)cam;
			cam = (float4x4_a&)(DirectX::XMMatrixMultiply((DirectX::XMMATRIX&)temp, am));
		}
		if (keypress[40] == true) //down
		{
			cam[3].xyz -= cam[2].xyz * perframespeed;
		}
	}
	void FREEaddingpart(emitter& emit) {
		int fpIndex = free.alloc();
		if (fpIndex != -1)
		{
			int spIndex = emit.indices.alloc();
			float3 vlc = float3(randomFloat(-1.9f, 1.9f), randomFloat(6.f, 14.f), randomFloat(-1.9f, 1.9f));
			if (spIndex != -1)
			{
				particle p = particle{ emit.spawn_pos, emit.spawn_color , vlc, emit.spawn_pos };
				free[fpIndex] = p;
				emit.indices[spIndex] = fpIndex;
			}
			else
			{
				free.free(fpIndex);
			}
		}
	}

	void FREEupdatePart(emitter& emit) {
		for (int i = 0; i < emit.indices.size(); i++)
		{
			particle& p = free[emit.indices[i]];
			if (p.pos.y >= 0)
			{
				p.prev_pos = p.pos;
				p.pos += p.vel * delta_time;
				p.vel.y += (grv * delta_time);
			}
			else
			{
				free.free(emit.indices[i]);
				//emit.indices.free(i);
			}
		}
	}

	void FreeDraw(emitter& emit) {

		FREEaddingpart(emit);
		FREEupdatePart(emit);
		for (int i = 0; i < emit.indices.size(); i++)
		{
			end::debug_renderer::add_line(free[emit.indices[i]].pos, free[emit.indices[i]].prev_pos, free[emit.indices[i]].clr);
		}
	}



	void sortedAddPart() {
		for (int i = 0; i < 256; i++)
		{
			int index = pool.alloc();

			float3 vlc = float3(randomFloat(-1.9f, 1.9f), randomFloat(5.f, 10.f), randomFloat(-1.9f, 1.9f));
			float3 pos = float3(0, 0, 0);
			float4 clr = float4(.8, .1, .9, 1);
			if (index != -1)
			{
				particle p = particle{ pos, clr , vlc ,pos };
				pool[index] = p;

			}
		}
	}

	void SOrtedUpdatePart() {
		for (int i = 0; i < pool.size(); i++)
		{
			particle& p = pool[i];
			if (p.pos.y < 0)
			{
				pool.free(i);
			}
			else
			{
				p.prev_pos = p.pos;
				p.pos += p.vel * delta_time;
				p.vel.y += (grv * delta_time);
			}
		}
	}

	void sortedDrawPart()
	{
		for (int i = 0; i < pool.size(); i++)
		{
			end::debug_renderer::add_line(pool[i].pos, pool[i].prev_pos, pool[i].clr);
		}
	}
	double dev_app_t::get_delta_time()const
	{
		return delta_time;
	}

	dev_app_t::dev_app_t()
	{
		std::cout << "WASD for moving matrix\n Arrow Keys for camera zoom in zoom out and rotating on Y\n Lbutton hold for camera movements in orthogonal oriented rotations with mouse movement"; // Don’t forget to include <iostream>
	}

	double calc_delta_time()
	{
		static std::chrono::time_point<std::chrono::high_resolution_clock> last_time = std::chrono::high_resolution_clock::now();

		std::chrono::time_point<std::chrono::high_resolution_clock> new_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed_seconds = new_time - last_time;
		last_time = new_time;

		return std::min(1.0 / 15.0, elapsed_seconds.count());
	}

	void getAbox(const aabb_t& box , const bool check) {
		float3 FTR, FTL, FBR, FBL, NTR, NTL, NBL, NBR;
		FTR = box.center + box.extents;
		FTL = { box.center.x - box.extents.x, box.center.y + box.extents.y, box.center.z + box.extents.z };
		FBL = { box.center.x - box.extents.x, box.center.y - box.extents.y, box.center.z + box.extents.z };
		FBR = { box.center.x + box.extents.x, box.center.y - box.extents.y, box.center.z + box.extents.z };

		NBL = box.center - box.extents;
		NTL = { box.center.x - box.extents.x, box.center.y + box.extents.y, box.center.z - box.extents.z };
		NTR = { box.center.x + box.extents.x, box.center.y + box.extents.y, box.center.z - box.extents.z };
		NBR = { box.center.x + box.extents.x, box.center.y - box.extents.y, box.center.z - box.extents.z };

		float4 ylw = { 1,1,0,1 };
		if (check == true)
		{
			ylw = { 1,0,0,1 };
		}

		end::debug_renderer::add_line(FTR, NBL, ylw);

		end::debug_renderer::add_line(FTR, FTL, ylw);
		end::debug_renderer::add_line(NTR, NTL, ylw);
		end::debug_renderer::add_line(NTR, FTR, ylw);
		end::debug_renderer::add_line(NTL, FTL, ylw);

		end::debug_renderer::add_line(FBR, FBL, ylw);
		end::debug_renderer::add_line(NBR, NBL, ylw);
		end::debug_renderer::add_line(FBR, NBR, ylw);
		end::debug_renderer::add_line(FBL, NBL, ylw);

		end::debug_renderer::add_line(FTR, FBR, ylw);
		end::debug_renderer::add_line(NTL, NBL, ylw);
		end::debug_renderer::add_line(FTL, FBL, ylw);
		end::debug_renderer::add_line(NTR, NBR, ylw);
	}

	void dev_app_t::update(float4x4_a& camera)
	{
		
		delta_time = calc_delta_time();
		elapse += delta_time;
		//This drawn the green checkmark
		/*end::debug_renderer::add_line(float3(-2, 0, 0), float3(0, -3, 0), float4(0.1f, 1, 0.1f, 1));
		end::debug_renderer::add_line(float3(0, -3, 0), float3(3, 4, 0), float4(0.1f, 1, 0.1f, 1));*/

		//TODO do you Updates here
		//sortedAddPart();


		if (elapse > 0.5f)
		{
			red = rand() % 100 / 100.0f;
			green = rand() % 100 / 100.0f;
			blue = rand() % 100 / 100.0f;
			elapse = 0.f;
		}

		for (int i = 8; i >= -8; i--)
		{
			end::debug_renderer::add_line(float3(8, 0, i), float3(-8, 0, i), float4(red, green, blue, 1));
			end::debug_renderer::add_line(float3(i, 0, 8), float3(i, 0, -8), float4(red, green, blue, 1));
		}

		/*SOrtedUpdatePart();


		sortedDrawPart();
		FreeDraw(yellowemit);
		FreeDraw(Bluemilk);
		FreeDraw(GreenRanger);
		FreeDraw(RedSchmit);*/

		Lookat[3].xyz = { 3,5,4 };
		rotater[3].xyz = { -3,5,4 };
		DrawAxis(Lookat);
		DrawAxis(at);
		DrawAxis(rotater);
		Lookat = (float4x4&)lookatTarget(Lookat[3].xyz, at[3].xyz, {0,1 , 0});

		keypressing();

		turntoTarget(rotater, at, .8f);
		mouselook(camera);
		worldmove(camera);
		v.view_mat = (float4x4_a&)at;
		calculate_frustum(frust, v);

		getAbox(box1, aabb_to_frustum(box1, frust));
		
		/*for (size_t i = 0; i < 256; i++)
		{
			if (keypress[i] == true)
			{
				std::cout << i << std::endl;
			}
		}*/
		
		
	}
}

namespace Game
{
#pragma once
	class ConvexHull3D
	{
#define MAXN 1010

		typedef long long vtype;

		/* Basic 3D vector implementation */
		struct vec3 {
			vec3() { X[0] = X[1] = X[2] = 0; }
			vec3(vtype x, vtype y, vtype z) { X[0] = x; X[1] = y; X[2] = z; }

			/* 3D cross product */
			vec3 operator*(const vec3& v) const {
				return vec3(X[1] * v.X[2] - X[2] * v.X[1],
					X[2] * v.X[0] - X[0] * v.X[2],
					X[0] * v.X[1] - X[1] * v.X[0]);
			}

			vec3 operator-(const vec3& v) const {
				return vec3(X[0] - v.X[0], X[1] - v.X[1], X[2] - v.X[2]);
			}

			vec3 operator-() const {
				return vec3(-X[0], -X[1], -X[2]);
			}

			vtype dot(const vec3& v) const {
				return X[0] * v.X[0] + X[1] * v.X[1] + X[2] * v.X[2];
			}

			vtype X[3];
		};
		/* Original points in the input. */
		vec3 A[MAXN];

		/* E[i][j] indicates which (up to two) other points combine with the edge i and
		* j to make a face in the hull.  Only defined when i < j.
		*/
		struct twoset {
			void insert(int x) { (a == -1 ? a : b) = x; }
			bool contains(int x) { return a == x || b == x; }
			void erase(int x) { (a == x ? a : b) = -1; }
			int size() { return (a != -1) + (b != -1); }
			int a, b;
		} E[MAXN][MAXN];

		struct face {
			vec3 norm;
			vtype disc;
			int I[3];
		};
		/* Compute the half plane {x : c^T norm < disc}
		* defined by the three points A[i], A[j], A[k] where
		* A[inside_i] is considered to be on the 'interior' side of the face. */
		face make_face(int i, int j, int k, int inside_i) {
			E[i][j].insert(k); E[i][k].insert(j); E[j][k].insert(i);

			face f;
			f.I[0] = i; f.I[1] = j; f.I[2] = k;
			f.norm = (A[j] - A[i]) * (A[k] - A[i]);
			f.disc = f.norm.dot(A[i]);
			if (f.norm.dot(A[inside_i]) > f.disc) {
				f.norm = -f.norm;
				f.disc = -f.disc;
			}
			return f;
		}

	public:
		ConvexHull3D();
		~ConvexHull3D();
	};
}

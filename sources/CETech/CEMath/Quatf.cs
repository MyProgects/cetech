using MoonSharp.Interpreter;

namespace CETech.CEMath
{
  [MoonSharpUserData]
  public struct Quatf
  {
    public static readonly Quatf Zero = new Quatf();
    public static readonly Quatf Identity = new Quatf(0.0f, 0.0f, 0.0f, 1.0f);

    public float X, Y, Z, W;

    public Quatf(float x, float y, float z, float w)
    {
      X = x;
      Y = y;
      Z = z;
      W = w;
    }

    public static Quatf make(float x, float y, float z, float w)
    {
      return new Quatf(x, y, z, w);
    }

    public static Mat4f ToMat4F(Quatf q)
    {
      var res = Mat4f.Identity;

      var x = q.X;
      var y = q.Y;
      var z = q.Z;
      var w = q.W;

      var x2 = x + x;
      var y2 = y + y;
      var z2 = z + z;
      var x2x = x2*x;
      var x2y = x2*y;
      var x2z = x2*z;
      var x2w = x2*w;
      var y2y = y2*y;
      var y2z = y2*z;
      var y2w = y2*w;
      var z2z = z2*z;
      var z2w = z2*w;

      res.M11 = 1.0f - (y2y + z2z);
      res.M12 = x2y - z2w;
      res.M13 = x2z + y2w;
      res.M14 = 0.0f;

      res.M21 = x2y + z2w;
      res.M22 = 1.0f - (x2x + z2z);
      res.M23 = y2z - x2w;
      res.M24 = 0.0f;

      res.M31 = x2z - y2w;
      res.M32 = y2z + x2w;
      res.M33 = 1.0f - (x2x + y2y);
      res.M34 = 0.0f;

      res.M41 = 0.0f;
      res.M42 = 0.0f;
      res.M43 = 0.0f;
      res.M44 = 1.0f;

      return res;
    }

    public static Quatf FromAxisAngle(Vec3f axis, float angle)
    {
        var angle_half = angle*0.5f;
        var sin = CMath.Sinf(angle_half);
        var cos = CMath.Cosf(angle_half);

        var a = Vec3f.Normalize(axis);

        return new Quatf(
            sin*a.X,
            sin*a.Y,
            sin*a.Z,
            cos
            );
    }

    public static Quatf FromEurelAngle(float heading, float attitude, float bank)
    {
            float sx = CMath.Sinf(heading *0.5f);
            float sy = CMath.Sinf(attitude * 0.5f);
            float sz = CMath.Sinf(bank * 0.5f);
            float cx = CMath.Cosf(heading * 0.5f);
            float cy = CMath.Cosf(attitude * 0.5f);
            float cz = CMath.Cosf(bank * 0.5f);

        return new Quatf(cx*cy*cz + sx*sy*sz,
            sx*cy*cz - cx*sy*sz,
            cx*sy*cz + sx*cy*sz,
            cx*cy*sz - sx*sy*cz);
    }

    public static Vec3f ToEurelAngle(Quatf q1)
    {
      float x = q1.X;
      float y = q1.Y;
      float z = q1.Z;
      float w = q1.W;

      float yy = y*y;
      float zz = z*z;
	  float xx = x*x;

      return new Vec3f(
          CMath.Atan2f(2.0f*(x*w - y*z), 1.0f - 2.0f*(xx + zz)),
          CMath.Atan2f(2.0f*(y*w + x*z), 1.0f - 2.0f*(yy + zz)),
          CMath.Asin(2.0f*(x*y + z*w))
      	);
    }

    public static float Length(Quatf q1)
    {
      return CMath.FastSqrtf(LengthSquared(q1));
    }

    public static float LengthSquared(Quatf q1)
    {
      return q1.X*q1.X + q1.Y*q1.Y + q1.Z*q1.Z + q1.W*q1.W;
    }

    public static float InvLength(Quatf q1)
    {
      return CMath.FastInvSqrtf(LengthSquared(q1));
    }

    public static Quatf Normalize(Quatf q1)
    {
      return q1*InvLength(q1);
    }

    public static bool IsNormalized(Quatf v1)
    {
      return CMath.Abs(1.0f - LengthSquared(v1)) < 0.001f;
    }

    public static Quatf operator -(Quatf q1)
    {
      //CE_ASSERT("quaternion", quat::IsNormalized(q1));

      return new Quatf(
        -q1.X,
        -q1.Y,
        -q1.Z,
        q1.W
        );
    }

    public static Quatf operator +(Quatf q1, Quatf q2)
    {
      return new Quatf(
        q1.X + q2.X,
        q1.Y + q2.Y,
        q1.Z + q2.Z,
        q1.W + q2.W
        );
    }

    public static Quatf operator -(Quatf q1, Quatf q2)
    {
      return new Quatf(
        q1.X - q2.X,
        q1.Y - q2.Y,
        q1.Z - q2.Z,
        q1.W - q2.W
        );
    }

    public static Quatf operator *(Quatf q1, Quatf q2)
    {
      var ax = q1.X;
      var ay = q1.Y;
      var az = q1.Z;
      var aw = q1.W;

      var bx = q2.X;
      var by = q2.Y;
      var bz = q2.Z;
      var bw = q2.W;

      return new Quatf(
        aw*bx + ax*bw + ay*bz - az*by,
        aw*by + ay*bw + az*bx - ax*bz,
        aw*bz + az*bw + ax*by - ay*bx,
        aw*bw - ax*bx - ay*by - az*bz
        );
    }

    public static Quatf operator *(Quatf q1, float s)
    {
      return new Quatf(
        q1.X*s,
        q1.Y*s,
        q1.Z*s,
        q1.W*s
        );
    }

    public static Vec3f operator *(Quatf q1, Vec3f v1)
    {
      var qv = new Vec3f(q1.X, q1.Y, q1.Z);

      var res = Vec3f.Cross(qv, v1)*(2.0f*q1.W);
      res = res + v1*(CMath.Square(q1.W) - Vec3f.Dot(qv, qv));
      res = res + qv*(2.0f*Vec3f.Dot(qv, v1));

      return res;
    }

    public static Quatf operator /(Quatf q1, float s)
    {
      var inv_s = 1.0f/s;

      return new Quatf(
        q1.X*inv_s,
        q1.Y*inv_s,
        q1.Z*inv_s,
        q1.W*inv_s
        );
    }
  }
}
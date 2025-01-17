using System;

namespace Louron
{
    public struct Quaternion
    {
        public float X, Y, Z, W;

        // Constructor for Quaternion from components
        public Quaternion(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        // Multiply two quaternions
        public static Quaternion operator *(Quaternion a, Quaternion b)
        {
            return new Quaternion(
                a.W * b.X + a.X * b.W + a.Y * b.Z - a.Z * b.Y,
                a.W * b.Y + a.Y * b.W + a.Z * b.X - a.X * b.Z,
                a.W * b.Z + a.Z * b.W + a.X * b.Y - a.Y * b.X,
                a.W * b.W - a.X * b.X - a.Y * b.Y - a.Z * b.Z
            );
        }

        // Create a quaternion from an axis-angle representation
        public static Quaternion CreateFromAxisAngle(Vector3 axis, float angle)
        {
            float halfAngle = angle * 0.5f;
            float s = MathF.Sin(halfAngle);
            return new Quaternion(axis.X * s, axis.Y * s, axis.Z * s, MathF.Cos(halfAngle));
        }
    }

}

using UnityEngine;

public class ClawController : MonoBehaviour
{
    public float moveSpeed = 2f;
    private Rigidbody rb;

    void Start()
    {
        rb = GetComponent<Rigidbody>();
        rb.constraints = RigidbodyConstraints.FreezeRotation; // prevent tipping
    }

    void FixedUpdate()
    {
        // WASD movement
        float h = 0f;
        float v = 0f;

        if (Input.GetKey(KeyCode.A)) h = -1f;
        if (Input.GetKey(KeyCode.D)) h = 1f;
        if (Input.GetKey(KeyCode.W)) v = 1f;
        if (Input.GetKey(KeyCode.S)) v = -1f;

        Vector3 move = new Vector3(h, 0, v) * moveSpeed * Time.fixedDeltaTime;
        rb.MovePosition(rb.position + move);

        // Drop / Retract
        if (Input.GetKey(KeyCode.Space)) // drop
            rb.MovePosition(rb.position + Vector3.down * moveSpeed * Time.fixedDeltaTime);

        if (Input.GetKey(KeyCode.LeftShift)) // retract
            rb.MovePosition(rb.position + Vector3.up * moveSpeed * Time.fixedDeltaTime);
    }
}
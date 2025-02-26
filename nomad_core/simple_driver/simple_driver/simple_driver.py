import rclpy
from rclpy.duration import Duration
from rclpy.node import Node
from sensor_msgs.msg import Joy

from can_device import CanHat
import can
import cantools
import time
from pprint import pprint

class SimpleDriver(Node):
  def __init__(self):
    super().__init__("simple_driver")


    self.declare_parameter("kaganiec_predkosci", 0.2)
    self.declare_parameter("turn_radius", 1)

    self.declare_parameter("can_db", "/home/lemonx/it/nomad_software/nomad_hardware/can_device/can_constants/can_messages/output/can.dbc")
    can_db  = self.get_parameter("can_db").value  
    
    # CanHat().init_can_interface()
    self.can_bus = can.interface.Bus('can0', bustype='socketcan', bitrate=1000000)
    self.get_logger().info(f"CAN db file: {can_db}")
    
    self.db = cantools.database.load_file(can_db)
    self.frame_to_msg = { msg.frame_id: msg for msg in self.db.messages} 
    
    self.sub_joint_states = self.create_subscription(
        Joy,
        '/joy',
        self.joy_callback,
        10
      )
    self.timer = self.create_timer(0.05, self.can_receive)

  def set_duty_cycle(self,dc_FR,dc_FL,dc_RR,dc_RL):
    msgFR = self.db.get_message_by_name(f'vesc_fright_set_duty')
    msgFL = self.db.get_message_by_name(f'vesc_fleft_set_duty')
    msgRR = self.db.get_message_by_name(f'vesc_rright_set_duty')
    msgRL = self.db.get_message_by_name(f'vesc_rleft_set_duty')
    data_msg = msgFR.encode({"duty_cycle": dc_FR})
    mFR = can.Message(arbitration_id=msgFR.frame_id, data=data_msg, is_extended_id=msgFR.is_extended_frame,is_remote_frame=False)
    data_msg = msgFL.encode({"duty_cycle": dc_FL})
    mFL = can.Message(arbitration_id=msgFL.frame_id, data=data_msg, is_extended_id=msgFL.is_extended_frame,is_remote_frame=False)
    data_msg = msgRR.encode({"duty_cycle": dc_RR})
    mRR = can.Message(arbitration_id=msgRR.frame_id, data=data_msg, is_extended_id=msgRR.is_extended_frame,is_remote_frame=False)
    data_msg = msgRL.encode({"duty_cycle": dc_RL})
    mRL = can.Message(arbitration_id=msgRL.frame_id, data=data_msg, is_extended_id=msgRL.is_extended_frame,is_remote_frame=False)
    
    self.can_bus.send(mFR)
    self.can_bus.send(mFL)
    self.can_bus.send(mRR)  
    self.can_bus.send(mRL)

  def decode_msg(self,recived_frame):
    if(recived_frame is None):
      return None, None
    msg = self.frame_to_msg.get(recived_frame.arbitration_id)
    if msg is not None:
      return msg.decode(recived_frame.data), msg.name
    return None, None
  
  def can_receive(self):
    recived_frame = self.can_bus.recv(0.1)
    data,name = self.decode_msg(recived_frame)
    # if data is not None:
    #   print(f"{name} {data}")
  
  def joy_callback(self, msg: Joy):
    x = msg.axes[0]
    y = msg.axes[1]
    length = (x**2 + y**2)**0.5
    if length > 1:
      length = 1
    turn_radius = self.get_parameter("turn_radius").value
    y = y * turn_radius

    if y > 0:
      correction_L = 1
      correction_R = 1 - y*2
    else:
      correction_L = 1 + y*2
      correction_R = 1
    velocity = length * x

    dc_FR = velocity * correction_R
    dc_RR = velocity * correction_R
    dc_FL = velocity * correction_L
    dc_RL = velocity * correction_L

    a = self.get_parameter("kaganiec_predkosci").value * 100000
    dc_FR = int(dc_FR * a) 
    dc_FL = int(dc_FL * a)
    dc_RR = int(dc_RR * a)
    dc_RL = int(dc_RL * a)
    self.get_logger().info(f"dc_FL: {dc_FL} dc_FR: {dc_FR}  dc_RL: {dc_RL} dc_RR: {dc_RR}")
    self.set_duty_cycle(dc_FR,dc_FL,dc_RR,dc_RL)
    pass

def main(args=None):
  rclpy.init(args=args)
  sdnode = SimpleDriver()
  try:
    rclpy.spin(sdnode)
  except KeyboardInterrupt:
    pass
  finally:
    sdnode.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
  main()
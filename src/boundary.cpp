#include <ros/ros.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/features/normal_3d.h>
#include <pcl/search/kdtree.h>
#include <pcl_conversions/pcl_conversions.h>
#include <sensor_msgs/PointCloud2.h>
#include <visualization_msgs/Marker.h>

ros::Publisher normals_pub;
ros::Publisher marker_pub;

void pointCloudCallback(const pcl::PCLPointCloud2ConstPtr& input_cloud_msg) {
    pcl::PointCloud<pcl::PointXYZ>::Ptr pcl_cloud(new pcl::PointCloud<pcl::PointXYZ>);
    pcl::fromPCLPointCloud2(*input_cloud_msg, *pcl_cloud);

    pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> ne;
    ne.setInputCloud(pcl_cloud);

    pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>());
    ne.setSearchMethod(tree);

    ne.setRadiusSearch(0.03);

    pcl::PointCloud<pcl::Normal>::Ptr normals(new pcl::PointCloud<pcl::Normal>);
    ne.compute(*normals);

    pcl::PointCloud<pcl::PointNormal>::Ptr pcl_normals(new pcl::PointCloud<pcl::PointNormal>);
    pcl::concatenateFields(*pcl_cloud, *normals, *pcl_normals);

    sensor_msgs::PointCloud2 normals_msg;
    pcl::toROSMsg(*pcl_normals, normals_msg);
    normals_msg.header = pcl_conversions::fromPCL(input_cloud_msg->header);
    normals_pub.publish(normals_msg);

    visualization_msgs::Marker arrow_marker;
    arrow_marker.header = pcl_conversions::fromPCL(input_cloud_msg->header);
    arrow_marker.ns = "normals";
    arrow_marker.type = visualization_msgs::Marker::ARROW;
    arrow_marker.action = visualization_msgs::Marker::ADD;

    for (size_t i = 0; i < pcl_normals->points.size(); ++i) {
        arrow_marker.id = static_cast<int>(i);
        arrow_marker.points.resize(2);
        arrow_marker.points[0].x = pcl_normals->points[i].x;
        arrow_marker.points[0].y = pcl_normals->points[i].y;
        arrow_marker.points[0].z = pcl_normals->points[i].z;
        arrow_marker.points[1].x = pcl_normals->points[i].x + pcl_normals->points[i].normal_x * 0.1;
        arrow_marker.points[1].y = pcl_normals->points[i].y + pcl_normals->points[i].normal_y * 0.1;
        arrow_marker.points[1].z = pcl_normals->points[i].z + pcl_normals->points[i].normal_z * 0.1;

        arrow_marker.scale.x = 0.01;
        arrow_marker.scale.y = 0.02;
        arrow_marker.scale.z = 0.0;
        arrow_marker.color.r = 1.0;
        arrow_marker.color.g = 0.0;
        arrow_marker.color.b = 0.0;
        arrow_marker.color.a = 1.0;

        marker_pub.publish(arrow_marker);
    }
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "normal_estimation_node");
    ros::NodeHandle nh;

    normals_pub = nh.advertise<sensor_msgs::PointCloud2>("transformed_normals_topic", 1);
    marker_pub = nh.advertise<visualization_msgs::Marker>("normals_marker_topic", 1);

    ros::Subscriber sub = nh.subscribe<pcl::PCLPointCloud2>(
        "/kinect/depth/points", 1, pointCloudCallback);

    ros::spin();
    return 0;
}
